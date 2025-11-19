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
    // Create semi-circle arc
    arc_ = lv_arc_create(lv_scr_act());
    lv_obj_set_size(arc_, 160, 160);
    lv_arc_set_angles(arc_, 180, 0);  // semi-circle
    lv_obj_set_style_arc_width(arc_, 12, 0);
    lv_obj_set_style_arc_color(arc_, lv_color_gray(), LV_PART_MAIN);
    lv_obj_set_style_arc_color(arc_, lv_color_green(), LV_PART_INDICATOR);
    lv_obj_align(arc_, LV_ALIGN_CENTER, 0, 20);

    // Create numeric label
    label_ = lv_label_create(lv_scr_act());
    lv_label_set_text_fmt(label_, "%d%%", dimmerValue / DIMMER_STEP);
    lv_obj_align(label_, arc_, LV_ALIGN_CENTER, 0, 0);
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
