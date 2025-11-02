#include "HomeScreen.hpp"
enum screen_color
{
   SCREEN_COLOR_BLACK = 0x000000,
   SCREEN_COLOR_WHITE = 0xFFFFFF,
   SCREEN_COLOR_RED   = 0xFF0000,
   SCREEN_COLOR_GREEN = 0x00FF00,
   SCREEN_COLOR_BLUE  = 0x0000FF
};

static enum screen_color colors[] = {
    SCREEN_COLOR_RED,
    SCREEN_COLOR_GREEN,
    SCREEN_COLOR_BLUE,
    SCREEN_COLOR_WHITE,
    SCREEN_COLOR_BLACK};
static int color_count = sizeof(colors) / sizeof(colors[0]);

void HomeScreen::Render()
{
    if (screen_ == nullptr) {
        return;
    }

    screen_->change_color(colors[currentColorIndex]);
}

void HomeScreen::handle_input_event(const InputDeviceType device_type, const struct input_event& event)
{
    if (device_type == InputDeviceType::BUTTON && event.type == EV_KEY && event.code == 't' && event.value == 1)
    {
        if (beeper_ != nullptr)
        {
            beeper_->play(100);
        }   
        
        // Move to the next color
        currentColorIndex = (currentColorIndex + 1) % color_count;
    }
}