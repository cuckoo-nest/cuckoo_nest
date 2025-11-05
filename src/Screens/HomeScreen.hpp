#pragma once

#include "ScreenBase.hpp"
#include "../ScreenManager.hpp"
#include "../HAL/Display.hpp"
#include "../HAL/Beeper.hpp"

class HomeScreen : public ScreenBase
{
public:
    HomeScreen(
        ScreenManager* screenManager,
        Display *screen,
        Beeper *beeper) : 
        ScreenBase(), 
        screenManager_(screenManager),
        display_(screen), 
        beeper_(beeper),
        nextScreen_(nullptr) {}

    virtual ~HomeScreen() = default;

    void Render() override;
    std::string TimeToString(time_t time);
    void handle_input_event(const   InputDeviceType device_type, const struct input_event &event) override;
    void SetNextScreen(ScreenBase* screen) {
        nextScreen_ = screen;
    }

private:
    int currentColorIndex = 0;
    Display *display_;
    Beeper *beeper_;
    ScreenManager *screenManager_;
    ScreenBase* nextScreen_;
};
