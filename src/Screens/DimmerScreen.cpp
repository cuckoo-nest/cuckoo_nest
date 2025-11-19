#include "DimmerScreen.hpp"
#include <string>


void DimmerScreen::Render()
{
    if (display_ == nullptr) {
        return;
    }

    display_->SetBackgroundColor(SCREEN_COLOR_BLACK);

    std::string dimmerValueString = std::to_string(dimmerValue / DIMMER_STEP) + "%";
    
    if (dimmerValue == 0)
    {
        dimmerValueString = "Off";
    }

    display_->DrawText(60, -40, "Name???", SCREEN_COLOR_WHITE, Font::FONT_H1);
    display_->DrawText(100, 100, dimmerValueString, SCREEN_COLOR_WHITE, Font::FONT_H2);

    int cx = 160;
    int cy= 160;
    int radius = 120;
    int thickness = 12;
    int startAngle = 0;
    
    int brightnessPercent = dimmerValue / DIMMER_STEP;
    int filledAngle = (brightnessPercent * 360) / 100;
    int endAngle = startAngle + filledAngle;

    display_->DrawArc(cx, cy, radius, startAngle, endAngle, SCREEN_COLOR_BLUE, thickness, 0x202020, true);
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
