#include "SwitchScreen.hpp"
#include "logger.h"

void SwitchScreen::Render()
{
    if (display_ == nullptr)
        return;

    display_->SetBackgroundColor(SCREEN_COLOR_BLACK);
    display_->DrawText(40, -100, GetName().substr(0,10), SCREEN_COLOR_WHITE, Font::FONT_H1);

    std::string buttonText;
    if (selectedOption == SelectedOption::TOGGLE)
        buttonText = "> ";
    else
        buttonText = "  ";

    if (switchState == SwitchState::OFF)
        buttonText += "On";
    else
        buttonText += "Off";

    display_->DrawText(60, 0, buttonText, SCREEN_COLOR_WHITE, Font::FONT_H2);

    std::string navText = (GetNextScreenId() != "" ? "Done" : "Back");
    buttonText = (selectedOption == SelectedOption::BACK) ? "> " + navText : "  " + navText;
    display_->DrawText(60, 20, buttonText, SCREEN_COLOR_WHITE, Font::FONT_H2);
}

void SwitchScreen::OnChangeFocus(bool focused)
{
    LOG_INFO_STREAM("OnChangeFocus focus " << focused);

    if(focused)
    {
        auto integrationSwitch = screenManager_->GetIntegrationContainer()->GetSwitchById(GetIntegrationId());

        if (integrationSwitch != nullptr)
        {
            switchState = (
                integrationSwitch->GetState() == IntegrationSwitchBase::SwitchState::ON
                ? SwitchScreen::SwitchState::ON // fully qualified, just to make it clear
                : SwitchScreen::SwitchState::OFF // fully qualified, just to make it clear
            );
            std::string str = (switchState == SwitchState::ON ? "On" : "Off");
            LOG_INFO_STREAM("OnChangeFocus switchState " << str);
        }
    }
    ScreenBase::OnChangeFocus(focused);
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
        Render();
    }

    if (device_type == InputDeviceType::BUTTON && event.type == EV_KEY && event.code == 't' && event.value == 1)
    {
        if (beeper_ != nullptr)
            beeper_->click();

        if (selectedOption == SelectedOption::TOGGLE)
        {
            if (GetIntegrationId() == "") {
                LOG_WARN_STREAM("No integration ID set for this SwitchScreen");
                return;
            }

            auto integrationContainer_ = screenManager_->GetIntegrationContainer();
            if (integrationContainer_ == nullptr) {
                LOG_ERROR_STREAM("No integration container available");
                return;
            }

            auto sw = integrationContainer_->GetSwitchById(GetIntegrationId());
            if (sw == nullptr) {
                LOG_ERROR_STREAM("No switch found for integration ID: \"" << GetIntegrationId() << "\"");
                return;
            }

            // Toggle the switch state
            if (switchState == SwitchState::OFF)
            {
                switchState = SwitchState::ON;
                LOG_INFO_STREAM("Switch \"" << GetName() << "\" turned ON");
                sw->TurnOn();
            }
            else
            {
                switchState = SwitchState::OFF;
                LOG_INFO_STREAM("Switch \"" << GetName() << "\" turned OFF");
                sw->TurnOff();
            }
            Render();
        }
        else if (selectedOption == SelectedOption::BACK)
        {
            // Navigate back to the previous screen

            if (GetNextScreenId() != "")
                screenManager_->GoToNextScreen(GetNextScreenId());
            else
                screenManager_->GoToPreviousScreen();
            selectedOption = SelectedOption::TOGGLE; // Reset selection
        }
    }
}