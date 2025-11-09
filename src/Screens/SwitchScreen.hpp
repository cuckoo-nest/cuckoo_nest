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
        ScreenManager* screenManager) : 
        ScreenBase(), 
        screenManager_(screenManager),
        display_(hal->display), 
        beeper_(hal->beeper),
        rotaryAccumulator(0),
        switchState(SwitchState::OFF),
        selectedOption(SelectedOption::TOGGLE)
    {}

    virtual ~SwitchScreen() = default;

    void Render() override;
    void handle_input_event(const InputDeviceType device_type, const struct input_event &event) override;
    
    const int GetIntegrationId() const {
        return integrationId_;
    }
    void SetIntegrationId(int id) {
        integrationId_ = id;
    }

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
    int integrationId_;


};