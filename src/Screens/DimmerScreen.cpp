#include "DimmerScreen.hpp"
#include <string>


void DimmerScreen::Render()
{
    if (display_ == nullptr) {
        return;
    }

    display_->SetBackgroundColor(SCREEN_COLOR_BLACK);

    std::string dimmerValueString = std::to_string(dimmerValue / DIMMER_STEP) + "%";

    display_->DrawText(60, 80, "Dimmer", SCREEN_COLOR_WHITE, 3);
    display_->DrawText(100, 140, dimmerValueString, SCREEN_COLOR_WHITE, 4);
    display_->Flush();

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

        screenManager_->GoToPreviousScreen();
    }
}