#pragma once

#include "ScreenBase.hpp"
#include "../ScreenManager.hpp"
#include "../HAL/HAL.hpp"

class HomeScreen : public ScreenBase
{
public:
    HomeScreen(HAL *hal, ScreenManager *screenManager) : 
        ScreenBase(), 
        screenManager_(screenManager),
        display_(hal->display),
        beeper_(hal->beeper),
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
