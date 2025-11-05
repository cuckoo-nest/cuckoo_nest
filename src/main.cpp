#include <stdio.h>
#include <unistd.h>
#include "linux/input.h"

#include "HAL/Beeper.hpp"
#include "HAL/Display.hpp"
#include "HAL/Inputs.hpp"

#include "Screens/HomeScreen.hpp"
#include "Screens/MenuScreen.hpp"
#include "Screens/DimmerScreen.hpp"
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

    menu_screen->AddMenuItem(MenuItem("On", nullptr, &menu_screen_callback_on));
    menu_screen->AddMenuItem(MenuItem("Off", nullptr, &menu_screen_callback_off));
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

void menu_screen_callback_on()
{
    std::cout << "Menu Screen: On selected\n";
}

void menu_screen_callback_off()
{
    std::cout << "Menu Screen: Off selected\n";
}
