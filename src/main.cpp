#include <stdio.h>
#include <unistd.h>
#include "linux/input.h"

#include "HAL/HAL.hpp"

#include "Screens/HomeScreen.hpp"
#include "Screens/MenuScreen.hpp"
#include "Screens/DimmerScreen.hpp"
#include "Screens/SwitchScreen.hpp"

#include "Integrations/IntegrationContainer.hpp"
#include "Integrations/ActionHomeAssistantService.hpp"
#include "ConfigurationReader.hpp"

#include <iostream>

// Function declarations
void change_screen_color();
void handle_input_event(const InputDeviceType device_type, const struct input_event &event);
void menu_screen_callback_on();
void menu_screen_callback_off();

static HAL hal;
static Beeper beeper("/dev/input/event0");
static Display screen("/dev/fb0");
static Inputs inputs("/dev/input/event2", "/dev/input/event1");
static IntegrationContainer integration_container;
static ScreenManager screen_manager(&hal, &integration_container);

int main()
{
    std::cout << "Cuckoo Hello\n";

    hal.beeper = &beeper;
    hal.display = &screen;
    hal.inputs = &inputs;

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

    ActionHomeAssistantService ha_service_g_light_on(
        config.get_home_assistant_token(""),
        "notused",
        config.get_home_assistant_base_url(""),
        "switch/turn_on",
        "switch.dining_room_spot_lights"
    );

    ActionHomeAssistantService ha_service_g_light_off(
        config.get_home_assistant_token(""),
        "notused",
        config.get_home_assistant_base_url(""),
        "switch/turn_off",
        "switch.dining_room_spot_lights"
    );

    ActionHomeAssistantService ha_service_oo_light_on(
        config.get_home_assistant_token(""),
        "notused",
        config.get_home_assistant_base_url(""),
        "switch/turn_on",
        "switch.garden_room_main_lights"
    );

    ActionHomeAssistantService ha_service_oo_light_off(
        config.get_home_assistant_token(""),
        "notused",
        config.get_home_assistant_base_url(""),
        "switch/turn_off",
        "switch.garden_room_main_lights"
    );

    ActionHomeAssistantService ha_service_oe_light_on(
        config.get_home_assistant_token(""),
        "notused",
        config.get_home_assistant_base_url(""),
        "switch/turn_on",
        "switch.garden_room_external_lights"
    );

    ActionHomeAssistantService ha_service_oe_light_off(
        config.get_home_assistant_token(""),
        "notused",
        config.get_home_assistant_base_url(""),
        "switch/turn_off",
        "switch.garden_room_external_lights"
    );




    if (!screen.initialize())
    {
        std::cerr << "Failed to initialize screen\n";
        return 1;
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
    while (1)
    {
        screen_manager.RenderCurrentScreen();
        sleep(1); // Sleep for 1 second - background thread handles input polling
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

    screen_manager.ProcessInputEvent(device_type, event);
    screen_manager.RenderCurrentScreen();
}
