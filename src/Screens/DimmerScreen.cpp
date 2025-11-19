#include "DimmerScreen.hpp"
#include <string>

DimmerScreen::DimmerScreen(HAL *hal, ScreenManager* screenManager)
    : screenManager_(screenManager),
      display_(hal->display),
      beeper_(hal->beeper),
      dimmerValue(50 * DIMMER_STEP)
{
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
}

void DimmerScreen::Render()
{
    if (!display_) return;

    display_->SetBackgroundColor(SCREEN_COLOR_BLACK);

    int brightnessPercent = dimmerValue / DIMMER_STEP;
    lv_arc_set_value(arc_, brightnessPercent);
    lv_label_set_text_fmt(label_, "%d%%", brightnessPercent);

    display_->TimerHandler();  // forces LVGL to redraw
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