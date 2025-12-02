#include "SwitchScreen.hpp"
#include "logger.h"

void SwitchScreen::Render()
{
    if (display_ == nullptr)
    {
        return;
    }

    display_->SetBackgroundColor(SCREEN_COLOR_BLACK);
    display_->DrawText(40, -100, GetName().substr(0,10), SCREEN_COLOR_WHITE, Font::FONT_H1);

    std::string buttonText;
    if (selectedOption == SelectedOption::TOGGLE)
    {
        buttonText = "> ";
    }

    if (switchState == SwitchState::OFF)
    {
        buttonText += "Turn on";
    }
    else
    {
        buttonText += "Turn off";
    }

    display_->DrawText(60, 0, buttonText, SCREEN_COLOR_WHITE, Font::FONT_H2);
    
    buttonText = (selectedOption == SelectedOption::BACK) ? "> Back" : "  Back";
    display_->DrawText(60, 20, buttonText, SCREEN_COLOR_WHITE, Font::FONT_H2);
}

void SwitchScreen::handle_input_event(const InputDeviceType device_type, const struct input_event &event)
{
    if (device_type == InputDeviceType::ROTARY)
    {
        rotaryAccumulator -= event.value;
        if (rotaryAccumulator >= 100)
        {
            selectedOption = SelectedOption::BACK;
            rotaryAccumulator = 0;
        }
        else if (rotaryAccumulator <= -100)
        {
            selectedOption = SelectedOption::TOGGLE;
            rotaryAccumulator = 0;
        }
    }

    if (device_type == InputDeviceType::BUTTON && event.type == EV_KEY && event.code == 't' && event.value == 1)
    {
        if (beeper_ != nullptr)
        {
            beeper_->play(100);
        }

        if (selectedOption == SelectedOption::TOGGLE)
        {
            if (integrationId_ == 0) {
                LOG_WARN_STREAM("No integration ID set for this SwitchScreen");
                return;
            }

            auto integrationContainer_ = screenManager_->GetIntegrationContainer();
            if (integrationContainer_ == nullptr) {
                LOG_ERROR_STREAM("No integration container available");
                return;
            }

            auto sw = integrationContainer_->GetSwitchById(integrationId_);
            if (sw == nullptr) {
                LOG_ERROR_STREAM("No switch found for integration ID: " << integrationId_);
                return;
            }

            // Toggle the switch state
            if (switchState == SwitchState::OFF)
            {
                switchState = SwitchState::ON;
                LOG_INFO_STREAM("Switch turned ON");
                sw->TurnOn();
            }
            else
            {
                switchState = SwitchState::OFF;
                LOG_INFO_STREAM("Switch turned OFF");
                sw->TurnOff();
            }

        }
        else if (selectedOption == SelectedOption::BACK)
        {
            // Navigate back to the previous screen
            screenManager_->GoToPreviousScreen();
            selectedOption = SelectedOption::TOGGLE; // Reset selection
        }
    }
}