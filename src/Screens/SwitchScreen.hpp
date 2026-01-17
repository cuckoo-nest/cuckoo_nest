#pragma once

#include "ScreenBase.hpp"
#include "../ScreenManager.hpp"

class SwitchScreen : public ScreenBase
{
public:
    SwitchScreen(
        ScreenManager* screenManager
        , const json11::Json &jsonConfig
    )
        : ScreenBase(screenManager, jsonConfig)
        , switchState(SwitchState::OFF)
        , selectedOption(SelectedOption::TOGGLE)
	{
        if(GetName() == "")
            SetName("Switch");
        // if no integration "id" attribute is provided, use our "name" attribute
        if(GetIntegrationId() == "")
            SetIntegrationId(GetName());

        beeper_ = screenManager_->HalBeeper();
        display_ = screenManager_->HalDisplay();
	}

    virtual ~SwitchScreen() = default;

    void Render() override;
    void handle_input_event(const InputDeviceType device_type, const struct input_event &event) override;
    void OnChangeFocus(bool focused) override;

    private:

    enum class SwitchState {
        OFF,
        ON
    }switchState;

    enum class SelectedOption {
        TOGGLE,
        BACK
    }selectedOption;

    Beeper* beeper_ = nullptr;
    IDisplay* display_ = nullptr;
    int rotaryAccumulator = 0;
};