#include "HomeScreen.hpp"

HomeScreen::HomeScreen(ScreenManager *screenManager, const json11::Json &jsonConfig)
    : ScreenBase(screenManager, jsonConfig)
{
}

void HomeScreen::OnChangeFocus(bool focused) 
{
}

void HomeScreen::Render()
{
}

void HomeScreen::handle_input_event(const InputDeviceType device_type, const struct input_event &event)
{
}