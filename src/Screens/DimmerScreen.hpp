#pragma once

#include "ScreenBase.hpp"
#include "../ScreenManager.hpp"
#include "../HAL/Display.hpp"
#include "../HAL/Beeper.hpp"

class DimmerScreen : public ScreenBase
{
public:
    DimmerScreen(
        ScreenManager* screenManager,
        Display *display,
        Beeper *beeper) : 
        ScreenBase(), 
        screenManager_(screenManager),
        display_(display), 
        beeper_(beeper),
        dimmerValue(50 * DIMMER_STEP) {}

    virtual ~DimmerScreen() = default;

    void Render() override;
    void handle_input_event(const InputDeviceType device_type, const struct input_event &event) override;

private:
    ScreenManager* screenManager_;
    Beeper* beeper_;
    Display* display_;
    int dimmerValue;

    const int DIMMER_STEP = 50; // step size for each rotary event
    const int MAX_DIMMER_VALUE = 100; // maximum dimmer value
    const int MIN_DIMMER_VALUE = 0;   // minimum dimmer value
};