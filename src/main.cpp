#include <stdio.h>
#include <unistd.h>
#include "linux/input.h"

#include "HAL/Beeper.hpp"
#include "HAL/Display.hpp"
#include "HAL/Inputs.hpp"

#include "Screens/HomeScreen.hpp"
#include "Screens/MenuScreen.hpp"
#include "Screens/DimmerScreen.hpp"

#include "Integrations/ActionHomeAssistantService.hpp"
#include "ConfigurationReader.hpp"

#include <iostream>

// Function declarations
void change_screen_color();
void handle_input_event(const InputDeviceType device_type, const struct input_event &event);
void menu_screen_callback_on();
void menu_screen_callback_off();

static Beeper beeper("/dev/input/event0");
static Display screen("/dev/fb0");
static Inputs inputs("/dev/input/event2", "/dev/input/event1");
static ScreenManager screen_manager;

int main()
{
    std::cout << "Cuckoo Hello\n";

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

    ActionHomeAssistantService ha_service_light_on(
        config.get_home_assistant_token(""),
        "notused",
        config.get_home_assistant_base_url(""),
        "switch/turn_on",
        config.get_home_assistant_entity_id("switch.dining_room_spot_lights")
    );

    ActionHomeAssistantService ha_service_light_off(
        config.get_home_assistant_token(""),
        "notused",
        config.get_home_assistant_base_url(""),
        "switch/turn_off",
        config.get_home_assistant_entity_id("switch.dining_room_spot_lights")
    );

    if (!screen.initialize())
    {
        std::cerr << "Failed to initialize screen\n";
        return 1;
    }

    auto menu_screen = new MenuScreen(
        &screen_manager,
        &screen,
        &beeper);

    auto home_screen = new HomeScreen(
        &screen_manager,
        &screen,
        &beeper);

    auto dimmer_screen = new DimmerScreen(
        &screen_manager,
        &screen,
        &beeper);

    menu_screen->AddMenuItem(MenuItem("On", nullptr, &ha_service_light_on));
    menu_screen->AddMenuItem(MenuItem("Off", nullptr, &ha_service_light_off));
    menu_screen->AddMenuItem(MenuItem("Dimmer", dimmer_screen, nullptr));
    menu_screen->AddMenuItem(MenuItem("Back", home_screen, nullptr));

    home_screen->SetNextScreen(menu_screen);

    screen_manager.GoToNextScreen(home_screen);

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
