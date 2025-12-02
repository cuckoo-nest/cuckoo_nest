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

#include "logger.h"
#include <queue>
#include <mutex>

#include <ctype.h>

#include "lvgl/lvgl.h"

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

// create a fifo for input events
std::queue<MyInputEvent> input_event_queue;
// Mutex for thread safety
std::mutex input_event_queue_mutex;

int main()
{
    std::cout << "Cuckoo Nest Starting Up..." << std::endl;

    setup_logging();    
    
    LOG_INFO_STREAM("Cuckoo starting up...");
    
    
    // ensure brightness is high on start up
    backlight.set_backlight_brightness(115);
    
    if (!screen.Initialize())
    {
        LOG_ERROR_STREAM("Failed to initialize screen");
        return 1;
    }
    
    LOG_INFO_STREAM("Screen initialized successfully");
    
    hal.beeper = &beeper;
    hal.display = &screen;
    hal.inputs = &inputs;
    hal.backlight = &backlight;

    // Load configuration
    ConfigurationReader config("config.json");
    if (config.load()) {
        LOG_INFO_STREAM("Configuration loaded successfully");
        LOG_INFO_STREAM("App name: " << config.get_string("app_name", "Unknown"));
        LOG_INFO_STREAM("Debug mode: " << (config.get_bool("debug_mode", false) ? "enabled" : "disabled"));
        LOG_INFO_STREAM("Max screens: " << config.get_int("max_screens", 5));
        
        // Home Assistant configuration
        if (config.has_home_assistant_config()) {
            LOG_INFO_STREAM("Home Assistant configured:");
            LOG_INFO_STREAM("  Base URL: " << config.get_home_assistant_base_url());
            LOG_INFO_STREAM("  Token: [configured]");
            LOG_INFO_STREAM("  Entity ID: " << config.get_home_assistant_entity_id());
        } else {
            LOG_INFO_STREAM("Home Assistant not configured");
        }
    } else {
        LOG_WARN_STREAM("Failed to load configuration, using defaults");
    }

    //return 0;
    
    integration_container.LoadIntegrationsFromConfig("config.json");
    screen_manager.LoadScreensFromConfig("config.json");
    screen_manager.GoToNextScreen(1);

    // Set up input event callback
    inputs.set_callback(handle_input_event);

    if (!inputs.start_polling())
    {
        LOG_ERROR_STREAM("Failed to start input polling");
        return 1;
    }

    LOG_INFO_STREAM("Input polling started in background thread...");

    // Main thread can now do other work or just wait
    int tick = 0;
    const int ticks_per_second = 1 * 1000 * 1000 / 5000; // 1 second / input polling interval (5ms)
    while (true)
    {
        {
            std::lock_guard<std::mutex> lock(input_event_queue_mutex);
            while (!input_event_queue.empty()) {
                LOG_DEBUG_STREAM("got event from queue");
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
    // Simple console-only setup.
    // Honor environment variable CUCKOO_LOG_LEVEL if present.
    cuckoo_log::Logger::set_level_from_env();
    // If CUCKOO_LOG_FILE is set, enable file logging (append)
    cuckoo_log::Logger::set_file_from_env();
    LOG_INFO_STREAM("Logging initialized (console" << (cuckoo_log::Logger::file_enabled() ? " + file" : "") << ")");
}

// Input event handler callback
void handle_input_event(const InputDeviceType device_type, const struct input_event &event)
{
    if (device_type == InputDeviceType::ROTARY && event.type == 0 && event.code == 0)
    {
        return; // Ignore "end of event" markers from rotary encoder
    }

    LOG_DEBUG_STREAM("Main: Received input event - type: " << event.type << ", code: " << event.code << ", value: " << event.value);

    std::lock_guard<std::mutex> lock(input_event_queue_mutex);
    input_event_queue.push(MyInputEvent(device_type, event));

    // screen_manager.ProcessInputEvent(device_type, event);
    // screen_manager.RenderCurrentScreen();
}
