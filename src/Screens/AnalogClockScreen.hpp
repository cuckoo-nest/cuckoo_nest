#pragma once

#include <ctime>

#include "ScreenBase.hpp"
#include "../ScreenManager.hpp"

#include "lvgl/lvgl.h"

class AnalogClockScreen : public ScreenBase
{
public:
    AnalogClockScreen(
            ScreenManager* screenManager
            , const json11::Json &jsonConfig
        )
        : ScreenBase(screenManager, jsonConfig)
	{
        beeper_ = screenManager_->HalBeeper();
        display_ = screenManager_->HalDisplay();
        if(GetName() == "")
            SetName("Analog Clock");
	}

    virtual ~AnalogClockScreen() = default;

    void handle_input_event(const InputDeviceType device_type, const struct input_event &event) override;
    void OnChangeFocus(bool focused) override;

private:
    Beeper* beeper_ = nullptr;
    IDisplay* display_ = nullptr;

    bool initialized_ = false;

    // LVGL objects
    lv_obj_t * clock_container = nullptr;
    lv_obj_t * face = nullptr;
    lv_obj_t * hour_hand = nullptr;
    lv_obj_t * minute_hand = nullptr;
    lv_obj_t * second_hand = nullptr;
    lv_obj_t * center_point = nullptr;

    // Timer for updating the clock
    lv_timer_t * clock_timer = nullptr;

    bool animating_ = false;

    static void update_clock_cb(lv_timer_t * timer);
    static void set_hand_rotation(void * obj, int32_t v);
    static void set_opacity(void * obj, int32_t v);
    static void anim_ready_cb(lv_anim_t * a);

    void UpdateClock();
    void CreateClockFace();
    void StartIntroAnimation();
};
