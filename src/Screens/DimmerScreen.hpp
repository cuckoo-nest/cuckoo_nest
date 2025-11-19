#pragma once

#include "ScreenBase.hpp"
#include "../ScreenManager.hpp"
#include "../HAL/IDisplay.hpp"
#include "../HAL/Beeper.hpp"
#include "lvgl/lvgl.h"

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

private:
    ScreenManager* screenManager_;
    Beeper* beeper_;
    IDisplay* display_;
    int dimmerValue;
    int integrationId_;

    const int DIMMER_STEP = 50; // step size for each rotary event
    const int MAX_DIMMER_VALUE = 100; // maximum dimmer value
    const int MIN_DIMMER_VALUE = 0;   // minimum dimmer value
    lv_obj_t* arc_;    // semi-circle gauge
    lv_obj_t* label_;  // numeric percentage
};
