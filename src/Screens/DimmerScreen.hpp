#pragma once

#include "ScreenBase.hpp"
#include "../ScreenManager.hpp"
#include "../HAL/IDisplay.hpp"
#include "../HAL/Beeper.hpp"

class DimmerScreen : public ScreenBase
{
public:
    DimmerScreen(HAL *hal, ScreenManager* screenManager) : 
        ScreenBase(), 
        screenManager_(screenManager),
        display_(hal->display), 
        beeper_(hal->beeper),
        dimmerValue(50 * DIMMER_STEP) {}

    virtual ~DimmerScreen() = default;

    void Render() override;
    void handle_input_event(const InputDeviceType device_type, const struct input_event &event) override;

    const int GetIntegrationId() const {
        return integrationId_;
    }
    void SetIntegrationId(int id) {
        integrationId_ = id;
    }
    const std::string& GetName() const {
        return name_;
    }
    void SetName(const std::string& name) {
        name_ = name;
    }

private:
    ScreenManager* screenManager_;
    Beeper* beeper_;
    IDisplay* display_;
    int dimmerValue;
    int integrationId_;
    std::string name_;

    const int DIMMER_STEP = 50; // step size for each rotary event
    const int MAX_DIMMER_VALUE = 100; // maximum dimmer value
    const int MIN_DIMMER_VALUE = 0;   // minimum dimmer value
};