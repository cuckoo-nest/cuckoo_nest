#include <iostream>
#include <cmath>

#include "AnalogClockScreen.hpp"
#include "logger.h"

#ifndef PI
#define PI 3.14159265358979323846
#endif

void AnalogClockScreen::OnChangeFocus(bool focused)
{
    if(focused)
    {
        display_->SetBackgroundColor(SCREEN_COLOR_BLACK); // Clears screen
        CreateClockFace();
        if (!clock_timer)
        {
            clock_timer = lv_timer_create(update_clock_cb, 250, this);
            StartIntroAnimation();
        }
    }
    else
    {
        if(clock_timer)
        {
            lv_timer_delete(clock_timer);
            clock_timer = nullptr;
        }
    }
}

void AnalogClockScreen::handle_input_event(const InputDeviceType device_type, const struct input_event &event)
{
    if (device_type == InputDeviceType::BUTTON && event.type == EV_KEY && event.code == 't' && event.value == 1)
    {
        if(beeper_ != nullptr)
            beeper_->click();

        if (GetNextScreenId() != "")
            screenManager_->GoToNextScreen(GetNextScreenId());
        else
            screenManager_->GoToPreviousScreen();
    }
}

void AnalogClockScreen::update_clock_cb(lv_timer_t * timer)
{
    AnalogClockScreen * screen = (AnalogClockScreen *)lv_timer_get_user_data(timer);
    screen->UpdateClock();
}

void AnalogClockScreen::set_hand_rotation(void * obj, int32_t v)
{
    lv_obj_set_style_transform_rotation((lv_obj_t*)obj, v, 0);
}

void AnalogClockScreen::set_opacity(void * obj, int32_t v)
{
    lv_obj_set_style_opa((lv_obj_t*)obj, v, 0);
}

void AnalogClockScreen::anim_ready_cb(lv_anim_t * a)
{
    AnalogClockScreen * screen = (AnalogClockScreen *)a->user_data;
    // Only one animation needs to clear the flag, or use a counter.
    // Since all end around same time, simply clearing it is fine, 
    // though it might cause slight jump if others aren't done.
    // Better: check if this is the last/longest animation.
    // Or just clear it.
    screen->animating_ = false;
}

void AnalogClockScreen::StartIntroAnimation()
{
    animating_ = true;

    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    struct tm * t = localtime(&ts.tv_sec);

    // Calculate target angles (0.1 deg units)
    // Add full rotations for effect
    // Estimate duration = 1500ms. Add 1.5s to target time to land mostly correct?
    // Or just animate to current and let it jump 1.5s (hardly noticeable).

    float sec_angle = (t->tm_sec + ts.tv_nsec / 1000000000.0f) * 6.0f;
    float min_angle = (t->tm_min + t->tm_sec / 60.0f) * 6.0f;
    float hour_angle = (t->tm_hour % 12 + t->tm_min / 60.0f) * 30.0f;

    int32_t s_target = (int32_t)(sec_angle * 10) + 3600 * 2; // 2 extra spins
    int32_t m_target = (int32_t)(min_angle * 10) + 3600;     // 1 extra spin
    int32_t h_target = (int32_t)(hour_angle * 10) + 3600;    // 1 extra spin

    // Hour Hand Anim
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, hour_hand);
    lv_anim_set_values(&a, 0, h_target);
    lv_anim_set_time(&a, 1000);
    lv_anim_set_exec_cb(&a, set_hand_rotation);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
    lv_anim_start(&a);

    // Minute Hand Anim
    lv_anim_set_var(&a, minute_hand);
    lv_anim_set_values(&a, 0, m_target);
    lv_anim_set_time(&a, 1200);
    lv_anim_start(&a);

    // Second Hand Anim
    lv_anim_set_var(&a, second_hand);
    lv_anim_set_values(&a, 0, s_target);
    lv_anim_set_time(&a, 1500);
    lv_anim_set_user_data(&a, this);
    lv_anim_set_completed_cb(&a, anim_ready_cb); // Only simplest/last callback needed
    lv_anim_start(&a);

    // Fade In Animation (Container)
    lv_anim_set_var(&a, clock_container);
    lv_anim_set_values(&a, 0, LV_OPA_COVER);
    lv_anim_set_time(&a, 800);
    lv_anim_set_path_cb(&a, lv_anim_path_linear);
    lv_anim_set_exec_cb(&a, set_opacity);
    // Clear previous callbacks to act only as fade
    lv_anim_set_completed_cb(&a, NULL);
    lv_anim_start(&a);
}

void AnalogClockScreen::CreateClockFace()
{
    // Create container for all clock elements
    clock_container = lv_obj_create(lv_scr_act());
    lv_obj_remove_style_all(clock_container);
    lv_obj_set_size(clock_container, LV_PCT(100), LV_PCT(100));
    lv_obj_center(clock_container);
    lv_obj_set_style_opa(clock_container, 0, 0); // Start transparent

    lv_obj_t * parent = clock_container;

    // Create face (circle)
    face = lv_scale_create(parent);
    lv_obj_set_size(face, 298, 298);
    lv_scale_set_mode(face, LV_SCALE_MODE_ROUND_INNER);
    lv_obj_set_style_bg_opa(face, LV_OPA_TRANSP, 0);
    lv_obj_set_style_line_width(face, 3, 0);
    lv_scale_set_label_show(face, true);
    lv_scale_set_total_tick_count(face, 61);
    lv_scale_set_major_tick_every(face, 5);
    lv_obj_set_style_bg_color(face, lv_color_black(), 0);

    static const char * custom_labels[] = {"12", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", NULL};
    lv_scale_set_text_src(face, custom_labels);
    lv_scale_set_range(face, 0, 12);
    lv_scale_set_angle_range(face, 360);
    lv_scale_set_rotation(face, 270);
    lv_obj_set_style_text_color(face, lv_color_white(), 0);
    lv_obj_center(face);

    // Hands (using basic objects instead of lines)
    // Use parent (screen) to avoid parenting issues with scale

    // Hour hand
    hour_hand = lv_obj_create(parent);
    lv_obj_set_size(hour_hand, 6, 60);
    lv_obj_set_style_bg_color(hour_hand, lv_color_white(), 0);
    lv_obj_set_style_radius(hour_hand, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_border_width(hour_hand, 0, 0);
    lv_obj_set_style_transform_pivot_x(hour_hand, lv_pct(50), 0);
    lv_obj_set_style_transform_pivot_y(hour_hand, lv_pct(100), 0);
    lv_obj_align(hour_hand, LV_ALIGN_CENTER, 0, -30);

    // Minute hand
    minute_hand = lv_obj_create(parent);
    lv_obj_set_size(minute_hand, 4, 90);
    lv_obj_set_style_bg_color(minute_hand, lv_color_white(), 0);
    lv_obj_set_style_radius(minute_hand, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_border_width(minute_hand, 0, 0);
    lv_obj_set_style_transform_pivot_x(minute_hand, lv_pct(50), 0);
    lv_obj_set_style_transform_pivot_y(minute_hand, lv_pct(100), 0);
    lv_obj_align(minute_hand, LV_ALIGN_CENTER, 0, -45);

    // Second hand
    second_hand = lv_obj_create(parent);
    lv_obj_set_size(second_hand, 2, 100);
    lv_obj_set_style_bg_color(second_hand, lv_palette_main(LV_PALETTE_RED), 0);
    lv_obj_set_style_radius(second_hand, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_border_width(second_hand, 0, 0);
    lv_obj_set_style_transform_pivot_x(second_hand, lv_pct(50), 0);
    lv_obj_set_style_transform_pivot_y(second_hand, lv_pct(100), 0);
    lv_obj_align(second_hand, LV_ALIGN_CENTER, 0, -50);

    // Center point
    center_point = lv_obj_create(parent);
    lv_obj_set_size(center_point, 12, 12);
    lv_obj_set_style_bg_color(center_point, lv_palette_main(LV_PALETTE_RED), 0);
    lv_obj_set_style_radius(center_point, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_border_width(center_point, 0, 0);
    lv_obj_center(center_point);
}

void AnalogClockScreen::UpdateClock()
{
    if (!hour_hand || !minute_hand || !second_hand) return;
    if (animating_) return;

    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    struct tm * t = localtime(&ts.tv_sec);

    // Calculate angles (0-360 degrees)
    // 12 o'clock = 0 degrees for calculations
    int sec_angle = t->tm_sec * 6;
    float min_angle = (t->tm_min + t->tm_sec / 60.0f) * 6.0f;
    float hour_angle = (t->tm_hour % 12 + t->tm_min / 60.0f) * 30.0f;

    int32_t s_rot = (int32_t)(sec_angle * 10);
    int32_t m_rot = (int32_t)(min_angle * 10);
    int32_t h_rot = (int32_t)(hour_angle * 10);

    // Rotate objects (Pivots and alignment set in CreateClockFace)
    lv_obj_set_style_transform_rotation(hour_hand, h_rot, 0);
    lv_obj_set_style_transform_rotation(minute_hand, m_rot, 0);
    lv_obj_set_style_transform_rotation(second_hand, s_rot, 0);
}
