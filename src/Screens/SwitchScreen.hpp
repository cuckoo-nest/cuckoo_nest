#pragma once

#include "ScreenBase.hpp"
#include "../ScreenManager.hpp"
#include "../HAL/HAL.hpp"
#include "../Integrations/IntegrationActionBase.hpp"

class SwitchScreen : public ScreenBase
{
public:
    SwitchScreen(
        HAL* hal,
        ScreenManager* screenManager,
        IntegrationActionBase* onAction,
        IntegrationActionBase* offAction) : 
        ScreenBase(), 
        screenManager_(screenManager),
        display_(hal->display), 
        beeper_(hal->beeper),
        onAction_(onAction),
        offAction_(offAction),
        rotaryAccumulator(0),
        switchState(SwitchState::OFF),
        selectedOption(SelectedOption::TOGGLE)
    {}

    virtual ~SwitchScreen() = default;

    void Render() override;
    void handle_input_event(const InputDeviceType device_type, const struct input_event &event) override;

private:

    enum class SwitchState {
        OFF,
        ON
    }switchState;

    enum class SelectedOption {
        TOGGLE,
        BACK
    }selectedOption;

    ScreenManager* screenManager_;
    Beeper* beeper_;
    Display* display_;
    int rotaryAccumulator;
    IntegrationActionBase* onAction_;
    IntegrationActionBase* offAction_;


};