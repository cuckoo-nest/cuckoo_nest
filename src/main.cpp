#include <unistd.h>
#include "linux/input.h"

#include "HAL/HAL.hpp"
#include "HAL/Display.hpp"

#include "Screens/HomeScreen.hpp"
#include "Screens/MenuScreen.hpp"
#include "Screens/DimmerScreen.hpp"
#include "Screens/SwitchScreen.hpp"

#include "Integrations/IntegrationContainer.hpp"
#include "Integrations/ActionHomeAssistantService.hpp"
#include "ConfigurationReader.hpp"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <queue>
#include <mutex>

#include <ctype.h>

#include "lvgl/lvgl.h"

#include "Backplate/UnixSerialPort.hpp"
#include "Backplate/BackplateComms.hpp"
#include "IDateTimeProvider.hpp"

class MyInputEvent{
public:
    MyInputEvent(InputDeviceType device_type, const struct input_event &event)
        : device_type(device_type), event(event) {}
    InputDeviceType device_type;
    struct input_event event;
};

/**
 * @brief Animation callback to set the size of an object.
 *
 * @param var The object to animate (the arc).
 * @param v The new value for width and height.
 */
static void anim_size_cb(void * var, int32_t v)
{
    lv_obj_t * obj = (lv_obj_t *)var; 
    lv_obj_set_size(obj, v, v);
    // Recenter the object as it grows/shrinks to keep it centered
    lv_obj_center(obj);
}

// Function declarations
static void setup_logging();
void handle_input_event(const InputDeviceType device_type, const struct input_event &event);

static HAL hal;
static Beeper beeper("/dev/input/event0");
static Display screen("/dev/fb0");
static Inputs inputs("/dev/input/event2", "/dev/input/event1");
static Backlight backlight("/sys/class/backlight/3-0036/brightness");
static IntegrationContainer integration_container;
static ScreenManager screen_manager(&hal, &integration_container);

// Backplate objects (leave them static so they live for program lifetime)
static UnixSerialPort backplateSerial("/dev/ttyO2");

// Simple system time provider implementation
class SystemDateTimeProvider : public IDateTimeProvider {
public:
    int gettimeofday(struct timeval &tv) override {
        return ::gettimeofday(&tv, nullptr);
    }
};

static SystemDateTimeProvider systemDateTimeProvider;
static BackplateComms backplateComms(&backplateSerial, &systemDateTimeProvider);

// create a fifo for input events
std::queue<MyInputEvent> input_event_queue;
// Mutex for thread safety
std::mutex input_event_queue_mutex;

int main()
{

    setup_logging();

    spdlog::info("Cuckoo starting up...");

    // ensure brightness is high on start up
    backlight.set_backlight_brightness(115);
    
    if (!screen.Initialize())
    {
        spdlog::error("Failed to initialize screen");
        return 1;
    }

    spdlog::info("Screen initialized successfully");

    hal.beeper = &beeper;
    hal.display = &screen;
    hal.inputs = &inputs;
    hal.backlight = &backlight;

    // Load configuration
    ConfigurationReader config("config.json");
    if (config.load()) {
        spdlog::info("Configuration loaded successfully");
        spdlog::info("App name: {}", config.get_string("app_name", "Unknown"));
        spdlog::info("Debug mode: {}", config.get_bool("debug_mode", false) ? "enabled" : "disabled");
        spdlog::info("Max screens: {}", config.get_int("max_screens", 5));
        
        // Home Assistant configuration
        if (config.has_home_assistant_config()) {
            spdlog::info("Home Assistant configured:");
            spdlog::info("  Base URL: {}", config.get_home_assistant_base_url());
            spdlog::info("  Token: [configured]");
            spdlog::info("  Entity ID: {}", config.get_home_assistant_entity_id());
        } else {
            spdlog::info("Home Assistant not configured");
        }
    } else {
        spdlog::warn("Failed to load configuration, using defaults");
    }
    
    integration_container.LoadIntegrationsFromConfig("config.json");
    screen_manager.LoadScreensFromConfig("config.json");
    screen_manager.GoToNextScreen(1);

    // Set up input event callback
    inputs.set_callback(handle_input_event);

    if (!inputs.start_polling())
    {
        spdlog::error("Failed to start input polling");
        return 1;
    }

    spdlog::info("Input polling started in background thread...");

    // Initialize backplate communications (this will start its worker thread)
    if (!backplateComms.Initialize()) {
        spdlog::error("Failed to initialize Backplate communications");
        // non-fatal: continue running without backplate comms
    } else {
        spdlog::info("Backplate communications initialized and running in background thread");
    }

    // Main thread can now do other work or just wait
    int tick = 0;
    const int ticks_per_second = 1 * 1000 * 1000 / 5000; // 1 second / input polling interval (5ms)
    while (true)
    {
        {
            std::lock_guard<std::mutex> lock(input_event_queue_mutex);
            while (!input_event_queue.empty()) {
                spdlog::debug("got event from queue");
                auto event = input_event_queue.front();
                input_event_queue.pop();
                screen_manager.ProcessInputEvent(event.device_type, event.event);
                screen_manager.RenderCurrentScreen();
            }
        }

        screen.TimerHandler();
        tick++;
        if (tick >= ticks_per_second)
        {
            tick = 0;
            // Do once-per-second tasks here if needed
            screen_manager.RenderCurrentScreen();
        }

        usleep(5000); // Sleep for 5ms
    }

    return 0;
}

static void setup_logging() 
{
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    std::vector<spdlog::sink_ptr> sinks {console_sink};
    
    // Try to create file sink, fall back to console-only if it fails
    try {
        auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("/var/log/cuckoo.log");
        file_sink->set_level(spdlog::level::info);
        sinks.push_back(file_sink);
    } catch (const spdlog::spdlog_ex&) {
        // Try fallback location
        try {
            auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("/tmp/cuckoo.log");
            file_sink->set_level(spdlog::level::info);
            sinks.push_back(file_sink);
        } catch (const spdlog::spdlog_ex&) {
            // Fall back to console-only logging
            // Logger will be set up with console sink only
        }
    }
    
    auto logger = std::make_shared<spdlog::logger>("cuckoo", sinks.begin(), sinks.end());
    
    spdlog::set_default_logger(logger);
    spdlog::flush_on(spdlog::level::info);
    
    // Check if file sink was successfully added (sinks.size() > 1 means console + file)
    if (sinks.size() > 1) {
        spdlog::info("Logging to console and file");
    } else {
        spdlog::warn("Logging to console only - failed to create log file");
    }
}

// Input event handler callback
void handle_input_event(const InputDeviceType device_type, const struct input_event &event)
{
    if (device_type == InputDeviceType::ROTARY && event.type == 0 && event.code == 0)
    {
        return; // Ignore "end of event" markers from rotary encoder
    }

    spdlog::debug("Main: Received input event - type: {}, code: {}, value: {}", 
                  event.type, event.code, event.value);

    std::lock_guard<std::mutex> lock(input_event_queue_mutex);
    input_event_queue.push(MyInputEvent(device_type, event));

    // screen_manager.ProcessInputEvent(device_type, event);
    // screen_manager.RenderCurrentScreen();
}
