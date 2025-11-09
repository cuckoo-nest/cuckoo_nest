#include "SwitchScreen.hpp"
#include <iostream>

void SwitchScreen::Render()
{
    if (display_ == nullptr)
    {
        return;
    }

    display_->SetBackgroundColor(SCREEN_COLOR_BLACK);

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

    display_->DrawText(60, 80, buttonText, SCREEN_COLOR_WHITE, 3);
    
    buttonText = (selectedOption == SelectedOption::BACK) ? "> Back" : "  Back";
    display_->DrawText(60, 120, buttonText, SCREEN_COLOR_WHITE, 3);

    display_->Flush();
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
                std::cout << "No integration ID set for this SwitchScreen\n";
                return;
            }

            auto integrationContainer_ = screenManager_->GetIntegrationContainer();
            if (integrationContainer_ == nullptr) {
                std::cout << "No integration container available\n";
                return;
            }

            auto sw = integrationContainer_->GetSwitchById(integrationId_);
            if (sw == nullptr) {
                std::cout << "No switch found for integration ID: " << integrationId_ << "\n";
                return;
            }

            // Toggle the switch state
            if (switchState == SwitchState::OFF)
            {
                switchState = SwitchState::ON;
                std::cout << "Switch turned ON\n";
                sw->TurnOn();
            }
            else
            {
                switchState = SwitchState::OFF;
                std::cout << "Switch turned OFF\n";
                sw->TurnOff();
            }

        }
        else if (selectedOption == SelectedOption::BACK)
        {
            // Navigate back to the previous screen
            screenManager_->GoToPreviousScreen();
        }
    }
}