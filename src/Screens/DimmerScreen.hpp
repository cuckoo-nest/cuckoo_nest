#pragma once

#include "ScreenBase.hpp"
#include "../ScreenManager.hpp"

class DimmerScreen : public ScreenBase
{
public:
    DimmerScreen(
        ScreenManager* screenManager
        , const json11::Json &jsonConfig
    )
        : ScreenBase(screenManager, jsonConfig)
	{
        beeper_ = screenManager_->HalBeeper();
        display_ = screenManager_->HalDisplay();
		dimmerValue_ = 50 * DIMMER_STEP;
        if(GetName() == "")
            SetName("Dimmer");
        // if no integration "id" attribute is provided, use our "name" attribute
        if(GetIntegrationId() == "")
            SetIntegrationId(GetName());
	}

    virtual ~DimmerScreen() = default;

    void Render() override;
    void handle_input_event(const InputDeviceType device_type, const struct input_event &event) override;
    void OnChangeFocus(bool focused) override;

private:
    Beeper* beeper_ = nullptr;
    IDisplay* display_ = nullptr;
    int dimmerValue_;
    enum class SwitchState {
        OFF,
        ON
    }switchState_;

    const int DIMMER_STEP = 50; // step size for each rotary event
    const int MAX_DIMMER_VALUE = 100; // maximum dimmer value
    const int MIN_DIMMER_VALUE = 0;   // minimum dimmer value
};