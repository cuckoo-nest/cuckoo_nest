#pragma once

#include "ScreenBase.hpp"
#include "../HAL/Screen.hpp"
#include "../HAL/Beeper.hpp"

class HomeScreen : public ScreenBase
{
public:
    HomeScreen(
        Screen *screen,
        Beeper *beeper) : screen_(screen), beeper_(beeper) {}
    void Render() override;
    void handle_input_event(const InputDeviceType device_type, const struct input_event& event) override;

private:
    int currentColorIndex = 0;
    Screen *screen_;
    Beeper *beeper_;
};
