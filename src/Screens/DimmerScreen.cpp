#include "DimmerScreen.hpp"
#include <string>
#include "lvgl/lvgl.h"

void DimmerScreen::Render()
{
    if (display_ == nullptr) {
        return;
    }

    display_->SetBackgroundColor(SCREEN_COLOR_BLACK);

    std::string dimmerValueString = std::to_string(dimmerValue / DIMMER_STEP) + "%";

    // Draw the text first
    display_->DrawText(60, -40, "Dimmer", SCREEN_COLOR_WHITE, Font::FONT_H1);
    display_->DrawText(100, 0, dimmerValueString, SCREEN_COLOR_WHITE, Font::FONT_H2);

    // --- Circular progress bar ---
    lv_obj_t* scr = lv_scr_act();

    static lv_obj_t* arc = nullptr;

    if (!arc) {
        arc = lv_arc_create(scr);
        lv_obj_remove_style_all(arc); // Remove default styles
        lv_arc_set_bg_angles(arc, 135, 405); // 270-degree arc starting from top-left (gap at bottom)
        lv_arc_set_angles(arc, 135, 135);    // Initial progress 0%
        lv_obj_set_size(arc, 120, 120);      // Adjust size as needed
        lv_obj_center(arc);

        // Style for the progress part
        static lv_style_t style_arc;
        lv_style_init(&style_arc);
        lv_style_set_line_width(&style_arc, 8);
        lv_style_set_line_color(&style_arc, lv_color_white());
        lv_style_set_line_rounded(&style_arc, true);
        lv_obj_add_style(arc, &style_arc, LV_PART_INDICATOR);
    }

    // Update progress dynamically
    int progress = dimmerValue / DIMMER_STEP; // 0 to 100%
    int angle = 135 + (progress * 270) / 100; // Map 0-100% to 270 degrees
    lv_arc_set_end_angle(arc, angle);
}

void DimmerScreen::handle_input_event(const InputDeviceType device_type, const struct input_event &event)
{
    if (device_type == InputDeviceType::ROTARY)
    {
        dimmerValue -= event.value;
        if (dimmerValue > MAX_DIMMER_VALUE * DIMMER_STEP)
            dimmerValue = MAX_DIMMER_VALUE * DIMMER_STEP;

        if (dimmerValue < MIN_DIMMER_VALUE)
            dimmerValue = MIN_DIMMER_VALUE;
    }

    if (device_type == InputDeviceType::BUTTON && event.type == EV_KEY && event.code == 't' && event.value == 1)
    {
        if (beeper_ != nullptr)
        {
            beeper_->play(100);
        }   
        
        auto dimmer = screenManager_->GetIntegrationContainer()->GetDimmerById(integrationId_);

        if (dimmer != nullptr)
        {
            int brightnessPercent = dimmerValue / DIMMER_STEP;
            dimmer->SetBrightness(brightnessPercent);
        }

        screenManager_->GoToPreviousScreen();
    }
}