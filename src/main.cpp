#include <stdio.h>
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

#include <iostream>
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
    std::cout << "Cuckoo Hello\n";

    // ensure brightness is high on start up
    backlight.set_backlight_brightness(115);
    
    if (!screen.Initialize())
    {
        std::cerr << "Failed to initialize screen\n";
        return 1;
    }

    std::cout << "Screen initialized successfully\n";

    hal.beeper = &beeper;
    hal.display = &screen;
    hal.inputs = &inputs;
    hal.backlight = &backlight;

    // Load configuration
    ConfigurationReader config("config.json");
    if (config.load()) {
        std::cout << "Configuration loaded successfully\n";
        std::cout << "App name: " << config.get_string("app_name", "Unknown") << "\n";
        std::cout << "Debug mode: " << (config.get_bool("debug_mode", false) ? "enabled" : "disabled") << "\n";
        std::cout << "Max screens: " << config.get_int("max_screens", 5) << "\n";
        
        // Home Assistant configuration
        if (config.has_home_assistant_config()) {
            std::cout << "Home Assistant configured:\n";
            std::cout << "  Base URL: " << config.get_home_assistant_base_url() << "\n";
            std::cout << "  Token: " << config.get_home_assistant_token().substr(0, 10) << "...\n";
            std::cout << "  Entity ID: " << config.get_home_assistant_entity_id() << "\n";
        } else {
            std::cout << "Home Assistant not configured\n";
        }
    } else {
        std::cout << "Failed to load configuration, using defaults\n";
    }
    
    integration_container.LoadIntegrationsFromConfig("config.json");
    screen_manager.LoadScreensFromConfig("config.json");
    screen_manager.GoToNextScreen(1);

    // Set up input event callback
    inputs.set_callback(handle_input_event);

    if (!inputs.start_polling())
    {
        std::cerr << "Failed to start input polling\n";
        return 1;
    }

    std::cout << "Input polling started in background thread...\n";

    // Main thread can now do other work or just wait
    int tick = 0;
    const int ticks_per_second = 1 * 1000 * 1000 / 5000; // 1 second / input polling interval (5ms)
    while (true)
    {
        {
            std::lock_guard<std::mutex> lock(input_event_queue_mutex);
            while (!input_event_queue.empty()) {
                std::cout << "got event from queue\n";
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

// Input event handler callback
void handle_input_event(const InputDeviceType device_type, const struct input_event &event)
{
    if (device_type == InputDeviceType::ROTARY && event.type == 0 && event.code == 0)
    {
        return; // Ignore "end of event" markers from rotary encoder
    }

    std::cout << "Main: Received input event - type: " << event.type 
              << ", code: " << event.code 
              << ", value: " << event.value << std::endl;

    std::lock_guard<std::mutex> lock(input_event_queue_mutex);
    input_event_queue.push(MyInputEvent(device_type, event));

    // screen_manager.ProcessInputEvent(device_type, event);
    // screen_manager.RenderCurrentScreen();
}
