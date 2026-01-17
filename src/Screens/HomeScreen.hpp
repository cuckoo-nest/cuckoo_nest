#pragma once

#include "ScreenBase.hpp"
#include "../ScreenManager.hpp"

#include "lvgl/lvgl.h"

class HomeScreen : public ScreenBase
{
public:
    HomeScreen(ScreenManager *screenManager, const json11::Json &jsonConfig);
    virtual ~HomeScreen() = default;

    void Render() override;
    std::string TimeToString(time_t time);
    std::string GetTemperatureString();
    std::string GetHumidityString();
    void handle_input_event(const InputDeviceType device_type, const struct input_event &event) override;
    void OnChangeFocus(bool focused) override;

private:
    int currentColorIndex = 0;
    Beeper* beeper_ = nullptr;
    IDisplay* display_ = nullptr;
    BackplateComms *backplateComms_ = nullptr;
    char temperatureUnits_ = 'c';
    int timeFormat_ = 24;

    static void timer_cb(lv_timer_t * timer);
    lv_timer_t * lvObjTimerCb = nullptr;
};
