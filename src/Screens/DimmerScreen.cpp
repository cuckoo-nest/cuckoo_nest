#include "DimmerScreen.hpp"
#include <string>


void DimmerScreen::Render()
{
    if (display_ == nullptr) {
        return;
    }

    display_->SetBackgroundColor(SCREEN_COLOR_BLACK);

    int brightnessPercent = dimmerValue / DIMMER_STEP;
    std::string dimmerValueString = std::to_string(brightnessPercent) + "%";

    // Draw the header text
    display_->DrawText(60, 80, "Dimmer", SCREEN_COLOR_WHITE, Font::FONT_H1);

    // Draw the brightness percentage
    display_->DrawText(100, 140, dimmerValueString, SCREEN_COLOR_WHITE, Font::FONT_H2);

    // Draw a simple horizontal brightness bar
    int barWidth = 200;
    int barHeight = 20;
    int filledWidth = (brightnessPercent * barWidth) / MAX_DIMMER_VALUE;

    // Draw the background of the bar
    display_->DrawText(60, 180, std::string(barWidth, '-'), SCREEN_COLOR_WHITE, Font::FONT_DEFAULT);

    // Draw the filled part of the bar
    display_->DrawText(60, 180, std::string(filledWidth, '='), SCREEN_COLOR_BLUE, Font::FONT_DEFAULT);
}

void DimmerScreen::handle_input_event(const InputDeviceType device_type, const struct input_event &event)
{
    if (device_type == InputDeviceType::ROTARY)
    {
        dimmerValue -= event.value;
        if (dimmerValue > MAX_DIMMER_VALUE * DIMMER_STEP)
            dimmerValue = MAX_DIMMER_VALUE * DIMMER_STEP;

        if (dimmerValue < MIN_DIMMER_VALUE)
            dimmerValue = MIN_DIMMER_VALUE;
    }

    if (device_type == InputDeviceType::BUTTON && event.type == EV_KEY && event.code == 't' && event.value == 1)
    {
        if (beeper_ != nullptr)
        {
            beeper_->play(100);
        }   
        
        auto dimmer = screenManager_->GetIntegrationContainer()->GetDimmerById(integrationId_);

        if (dimmer != nullptr)
        {
            int brightnessPercent = dimmerValue / DIMMER_STEP;
            dimmer->SetBrightness(brightnessPercent);
        }

        screenManager_->GoToPreviousScreen();
    }
}