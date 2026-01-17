#include <string>
#include "DimmerScreen.hpp"
#include "logger.h"

void DimmerScreen::Render()
{
    if (display_ == nullptr)
        return;

    display_->SetBackgroundColor(SCREEN_COLOR_BLACK);
    std::string dimmerValueString = std::to_string(dimmerValue_ / DIMMER_STEP) + "%";

	display_->DrawText(60, 80, GetName(), SCREEN_COLOR_WHITE, Font::FONT_H1);
    display_->DrawText(100, 0, dimmerValueString, SCREEN_COLOR_WHITE, Font::FONT_H2);
}

void DimmerScreen::OnChangeFocus(bool focused)
{
    LOG_INFO_STREAM("OnChangeFocus focus " << focused);

    if(focused)
    {
        auto integrationId = GetIntegrationId();
        // auto integrationSwitch = screenManager_->GetIntegrationContainer()->GetDimmerById(integrationId);
        auto dimmer = screenManager_->GetIntegrationContainer()->GetDimmerById(integrationId);

        if(dimmer != nullptr)
            dimmerValue_ = dimmer->GetBrightness() * DIMMER_STEP;

        if (dimmer != nullptr)
        {
            switchState_ = (
                dimmer->GetState() == IntegrationSwitchBase::SwitchState::ON
                ? DimmerScreen::SwitchState::ON // fully qualified, just to make it clear
                : DimmerScreen::SwitchState::OFF // fully qualified, just to make it clear
            );
            std::string str = (switchState_ == DimmerScreen::SwitchState::ON ? "On" : "Off");
            LOG_INFO_STREAM("OnChangeFocus switchState " << str);
        }
    }
    ScreenBase::OnChangeFocus(focused);
}

void DimmerScreen::handle_input_event(const InputDeviceType device_type, const struct input_event &event)
{
    if (device_type == InputDeviceType::ROTARY)
    {
        dimmerValue_ -= event.value;
        if (dimmerValue_ > MAX_DIMMER_VALUE * DIMMER_STEP)
            dimmerValue_ = MAX_DIMMER_VALUE * DIMMER_STEP;

        if (dimmerValue_ < MIN_DIMMER_VALUE)
            dimmerValue_ = MIN_DIMMER_VALUE;
        Render();
    }

    if (device_type == InputDeviceType::BUTTON && event.type == EV_KEY && event.code == 't' && event.value == 1)
    {
        if(beeper_ != nullptr)
            beeper_->click();
        
        auto dimmer = screenManager_->GetIntegrationContainer()->GetDimmerById(GetIntegrationId());

        if (dimmer != nullptr)
        {
            int brightnessPercent = dimmerValue_ / DIMMER_STEP;
            dimmer->SetBrightness(brightnessPercent);
        }

        if (GetNextScreenId() != "")
            screenManager_->GoToNextScreen(GetNextScreenId());
        else
            screenManager_->GoToPreviousScreen();
    }
}