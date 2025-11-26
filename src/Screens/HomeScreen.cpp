#include "HomeScreen.hpp"
#include <string>
#include <spdlog/spdlog.h>
#include "DimmerScreen.hpp"

static enum screen_color colors[] = {
    SCREEN_COLOR_BLACK,
    SCREEN_COLOR_RED,
    SCREEN_COLOR_GREEN,
    SCREEN_COLOR_BLUE,
    SCREEN_COLOR_WHITE,
};
static int color_count = sizeof(colors) / sizeof(colors[0]);

void HomeScreen::Render()
{
    if (display_ == nullptr) {
        return;
    }

    display_->SetBackgroundColor(colors[currentColorIndex]);

    //display_->DrawLine(0, 160, 320, 160, SCREEN_COLOR_WHITE);
    
    // Display text using the bitmap font
    uint32_t text_color = SCREEN_COLOR_WHITE;
    if (colors[currentColorIndex] == SCREEN_COLOR_WHITE) {
        text_color = SCREEN_COLOR_BLACK; // Use black text on white background
    }

    // get current time
    time_t now = time(0);
    display_->DrawText(60, -40, "Home", SCREEN_COLOR_WHITE, Font::FONT_H1);
    display_->DrawText(40, 0, TimeToString(now), text_color, Font::FONT_H1);
}

std::string HomeScreen::TimeToString(time_t time)
{
    char buffer[100];
    struct tm *timeinfo = localtime(&time);
    strftime(buffer, sizeof(buffer), "%H:%M:%S", timeinfo);
    return std::string(buffer);
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
        //currentColorIndex = (currentColorIndex + 1) % color_count;

        if (nextScreenId_ != -1)
        {
            screenManager_->GoToNextScreen(nextScreenId_);
        }
        else
        {
            spdlog::warn("Next screen is null!");
        }
    }
}