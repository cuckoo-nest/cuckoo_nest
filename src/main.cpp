#include <unistd.h>
#include "HAL/InputEvent.hpp"

#include "HAL/HAL.hpp"
#include "HAL/Display.hpp"

#include "Screens/HomeScreen.hpp"
#include "Screens/MenuScreen.hpp"
#include "Screens/DimmerScreen.hpp"
#include "Screens/SwitchScreen.hpp"

#include "Integrations/IntegrationContainer.hpp"
#include "Integrations/ActionHomeAssistantService.hpp"

#include "logger.h"
#include <queue>
#include <mutex>
#include <memory>
#include <fstream>
#include <sstream>
#include <json11.hpp>

#include <ctype.h>

#include "lvgl/lvgl.h"

#include "Backplate/UnixSerialPort.hpp"
#include "Backplate/BackplateComms.hpp"
#include "InputEvent.hpp"
#include "IDateTimeProvider.hpp"
#include "SystemDateTimeProvider.hpp"

#if defined(LV_USE_SDL) && LV_USE_SDL == 1
#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#endif

const int PROXIMITY_THRESHOLD = 3; // example threshold value

// HAL Configuration structure
struct HALConfig {
    std::string beeper_device;
    std::string display_device;
    std::string button_device;
    std::string rotary_device;
    std::string backlight_device;
    std::string backplate_serial_device;
    bool emulate_display;
    int backlight_active_seconds;
    int backlight_max_brightness;
    int backlight_min_brightness;
};

// Function declarations
static void setup_logging();
static HALConfig load_hal_config(const std::string& config_file);
void handle_input_event(const InputDeviceType device_type, const struct input_event &event);
void ProximityCallback(int value);

// Global pointers to HAL objects (will be initialized after config loading)
static std::unique_ptr<UnixSerialPort> backplateSerial;
static std::unique_ptr<SystemDateTimeProvider> systemDateTimeProvider;
static std::unique_ptr<BackplateComms> backplateComms;
static std::unique_ptr<HAL> hal;
static std::unique_ptr<Beeper> beeper;
static std::unique_ptr<Display> screen;
static std::unique_ptr<Inputs> inputs;
static std::unique_ptr<Backlight> backlight;
static std::unique_ptr<IntegrationContainer> integration_container;
static std::unique_ptr<ScreenManager> screen_manager;

// create a fifo for input events
std::queue<InputEvent> input_event_queue;
// Mutex for thread safety
std::mutex input_event_queue_mutex;


int main(int argc, char* argv[])
{
    std::cout << "Cuckoo Nest Starting Up..." << std::endl;

    setup_logging();    
    
    LOG_INFO_STREAM("Cuckoo starting up...");

    std::string config_file = "config.json";
    if (argc > 1) {
        config_file = argv[1];
    }

    
    // Load HAL configuration from config file
    HALConfig hal_config = load_hal_config(config_file);
    
    if (hal_config.emulate_display) {
        LOG_INFO_STREAM("Running in display emulation mode");
    }
    else {
        LOG_INFO_STREAM("Running on embedded target mode");
    }
    
    // Create HAL objects with configuration
    LOG_INFO_STREAM("Initializing HAL components...");
    beeper.reset(new Beeper(hal_config.beeper_device));
    screen.reset(new Display(hal_config.display_device));
    inputs.reset(new Inputs(hal_config.button_device, hal_config.rotary_device));
    backlight.reset(new Backlight(hal_config.backlight_device));
    
    // Initialize backplate communication components
    backplateSerial.reset(new UnixSerialPort(hal_config.backplate_serial_device));
    systemDateTimeProvider.reset(new SystemDateTimeProvider());
    backplateComms.reset(new BackplateComms(backplateSerial.get(), systemDateTimeProvider.get()));
    
    // Create HAL structure and containers
    hal.reset(new HAL());
    integration_container.reset(new IntegrationContainer());
    screen_manager.reset(new ScreenManager(hal.get(), integration_container.get(), backplateComms.get()));
    
    // Configure backlight with loaded settings
    backlight->set_active_seconds(hal_config.backlight_active_seconds);
    backlight->set_max_brightness(hal_config.backlight_max_brightness);
    backlight->set_min_brightness(hal_config.backlight_min_brightness);
    backlight->Activate();
    
    if (!screen->Initialize(hal_config.emulate_display))
    {
        LOG_ERROR_STREAM("Failed to initialize screen");
        return 1;
    }
    
    LOG_INFO_STREAM("Screen initialized successfully");
    
    hal->beeper = beeper.get();
    hal->display = screen.get();
    hal->inputs = inputs.get();
    hal->backlight = backlight.get();

    //return 0;
    
    integration_container->LoadIntegrationsFromConfig(config_file);
    screen_manager->LoadScreensFromConfig(config_file);
    screen_manager->GoToNextScreen(1);

    // Set up input event callback
    inputs->set_callback(handle_input_event);
    if (!hal_config.emulate_display)
    {
        if (!inputs->start_polling())
        {
            LOG_ERROR_STREAM("Failed to start input polling");
            return 1;
        }
    }

    LOG_INFO_STREAM("Input polling started in background thread...");

    backplateComms->AddPIRCallback(ProximityCallback);
    // Initialize backplate communications (this will start its worker thread)
    if (!backplateComms->Initialize()) {
        LOG_ERROR_STREAM("Failed to initialize Backplate communications");
        // non-fatal: continue running without backplate comms
    } else {
        LOG_INFO_STREAM("Backplate communications initialized and running in background thread");
    }

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
                screen_manager->ProcessInputEvent(event.device_type, event.event);
                screen_manager->RenderCurrentScreen();
            }
        }

        screen->TimerHandler();
        tick++;
        if (tick >= ticks_per_second)
        {
            tick = 0;
            // Do once-per-second tasks here if needed
            // Let Backlight manage its own timeout
            backlight->Tick();
        }

#if defined(LV_USE_SDL) && LV_USE_SDL == 1
        {
            SDL_Event e;
            while (SDL_PollEvent(&e))
            {
                switch (e.type)
                {
                case SDL_QUIT:
                    exit(0);
                    break;

                default:
                    LOG_DEBUG_STREAM("SDL event not handled " << e.type);
                    break;
                case SDL_MOUSEWHEEL:

                    // LOG_DEBUG_STREAM("mouse wheel to " << e.wheel.y);
                    handle_input_event(InputDeviceType::ROTARY, {.type = 1, .code = 1, .value = e.wheel.y * 10});
                    break;
                case SDL_MOUSEMOTION:
                    // LOG_DEBUG_STREAM("mouse motion to (" << e.motion.x << ", " << e.motion.y << ")");
                    break;
                case SDL_MOUSEBUTTONDOWN:
                {
                    SDL_MouseButtonEvent be = *(SDL_MouseButtonEvent *)(&e);
                    LOG_DEBUG_STREAM("mouse button down, clicks: " << be.clicks);
                    if (be.clicks == 1)
                        handle_input_event(InputDeviceType::BUTTON, {.type = EV_KEY, .code = 't', .value = 1});
                }
                break;
                }
            }
        }
        // #endif

        usleep(1000); // Sleep for 1ms
#else
        usleep(5000); // Sleep for 5ms
#endif
    }

    return 0;
}

static void setup_logging()
{
    // Simple console-only setup.
    // Honor environment variable CUCKOO_LOG_LEVEL if present.
    //cuckoo_log::Logger::set_level(cuckoo_log::Level::Debug);
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
        // Ignore 'end of event' markers from rotary encoder
        return;
    }

    LOG_DEBUG_STREAM("Main: Received input event - type: " << event.type << ", code: " << event.code << ", value: " << event.value);

    // Keep the screen bright on any input
    backlight->Activate();

    std::lock_guard<std::mutex> lock(input_event_queue_mutex);
    input_event_queue.push(InputEvent(device_type, event));
}

void ProximityCallback(int value)
{
    if (value >= PROXIMITY_THRESHOLD)
    {
        // PIR proximity should keep the backlight active
        backlight->Activate();
    }
}

// Load HAL configuration from JSON file with sensible defaults
static HALConfig load_hal_config(const std::string& config_file)
{
    HALConfig config;
    
    // Set default values for embedded target
    config.beeper_device = "/dev/input/event0";
    config.display_device = "/dev/fb0";
    config.button_device = "/dev/input/event2";
    config.rotary_device = "/dev/input/event1";
    config.backlight_device = "/sys/class/backlight/3-0036/brightness";
    config.backplate_serial_device = "/dev/ttyO2";
    config.emulate_display = false;
    config.backlight_active_seconds = 10;
    config.backlight_max_brightness = 115;
    config.backlight_min_brightness = 20;
    
    // Try to load configuration from file
    std::ifstream file(config_file);
    if (!file.is_open()) {
        LOG_WARN_STREAM("Could not open " << config_file << ", using default HAL configuration");
        return config;
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string configContent = buffer.str();
    
    if (configContent.empty()) {
        LOG_ERROR_STREAM("Config file " << config_file << " is empty, using defaults");
        return config;
    }

    std::string parse_error;
    json11::Json parsed_json = json11::Json::parse(configContent, parse_error);

    if (!parse_error.empty()) {
        LOG_ERROR_STREAM("JSON parse error: " << parse_error << ", using defaults");
        return config;
    }
    
    if (!parsed_json.is_object()) {
        LOG_ERROR_STREAM("Root JSON element must be an object, using defaults");
        return config;
    }
    
    LOG_INFO_STREAM("Loading HAL configuration from " << config_file);
    
    // Override defaults with values from config file if present
    auto hal = parsed_json["hal"];
    if (hal.is_object()) {
        if (hal["beeper_device"].is_string()) {
            config.beeper_device = hal["beeper_device"].string_value();
            LOG_DEBUG_STREAM("  beeper_device: " << config.beeper_device);
        }
        if (hal["display_device"].is_string()) {
            config.display_device = hal["display_device"].string_value();
            LOG_DEBUG_STREAM("  display_device: " << config.display_device);
        }
        if (hal["button_device"].is_string()) {
            config.button_device = hal["button_device"].string_value();
            LOG_DEBUG_STREAM("  button_device: " << config.button_device);
        }
        if (hal["rotary_device"].is_string()) {
            config.rotary_device = hal["rotary_device"].string_value();
            LOG_DEBUG_STREAM("  rotary_device: " << config.rotary_device);
        }
        if (hal["backlight_device"].is_string()) {
            config.backlight_device = hal["backlight_device"].string_value();
            LOG_DEBUG_STREAM("  backlight_device: " << config.backlight_device);
        }
        if (hal["backplate_serial_device"].is_string()) {
            config.backplate_serial_device = hal["backplate_serial_device"].string_value();
            LOG_DEBUG_STREAM("  backplate_serial_device: " << config.backplate_serial_device);
        }
        if (hal["emulate_display"].is_bool()) {
            config.emulate_display = hal["emulate_display"].bool_value();
            LOG_INFO_STREAM("  emulate_display: " << (config.emulate_display ? "true" : "false"));
        }
        if (hal["backlight_active_seconds"].is_number()) {
            config.backlight_active_seconds = hal["backlight_active_seconds"].int_value();
            LOG_DEBUG_STREAM("  backlight_active_seconds: " << config.backlight_active_seconds);
        }
        if (hal["backlight_max_brightness"].is_number()) {
            config.backlight_max_brightness = hal["backlight_max_brightness"].int_value();
            LOG_DEBUG_STREAM("  backlight_max_brightness: " << config.backlight_max_brightness);
        }
        if (hal["backlight_min_brightness"].is_number()) {
            config.backlight_min_brightness = hal["backlight_min_brightness"].int_value();
            LOG_DEBUG_STREAM("  backlight_min_brightness: " << config.backlight_min_brightness);
        }
    } else {
        LOG_WARN_STREAM("No 'hal' section found in config, using defaults");
    }
    
    return config;
}
