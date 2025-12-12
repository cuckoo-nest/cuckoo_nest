#include "MenuScreen.hpp"
#include "logger.h"
#include <lvgl/lvgl.h>
#include <cmath>

void MenuScreen::Render()
{
    if (display_ == nullptr)
    {
        return;
    }

    
    display_->SetBackgroundColor(SCREEN_COLOR_BLACK);

    //display_->DrawText(60, -100, "MenuName", SCREEN_COLOR_WHITE, Font::FONT_H1);
    
    std::string selectdMenu = "No ITems";
    if (!menuItems.empty())
    {
        selectdMenu = menuItems[menuSelectedIndex].name;
    } 
    display_->DrawText(0, 0, selectdMenu, SCREEN_COLOR_WHITE, Font::FONT_H2);
        
    // Draw circular icons representing each menu item using LVGL directly.
    // Icons are placed around a circle; the selected item is rotated to the top.
    const int screenW = 320;
    const int screenH = 320;
    const int centerX = screenW / 2;
    const int centerY = screenH / 2;
    const int radius = 120;

    int count = static_cast<int>(menuItems.size());
    if (count <= 0)
    {
        return;
    }

    const double angleStep = 360.0 / count;
    // rotate so the selected index is at -90 degrees (top)
    // incorporate rotaryAccumulator for fractional rotation between items
    double frac = 0.0;
    if (std::abs(rotaryAccumulator) > 10)
    {
        frac = static_cast<double>(rotaryAccumulator) / 100.0; // accumulator threshold is 100
        if (frac > 0.99) frac = 0.99;
        if (frac < -0.99) frac = -0.99;
    }

    // Prevent rotation beyond list bounds: when at first or last item, disallow
    // fractional movement that would visually spin icons past the ends.
    if (menuSelectedIndex <= 0 && frac < 0.0)
    {
        frac = 0.0;
    }
    if (menuSelectedIndex >= count - 1 && frac > 0.0)
    {
        frac = 0.0;
    }

    const double baseAngle = -90.0 - ((static_cast<double>(menuSelectedIndex) + frac) * angleStep);

    LOG_ERROR_STREAM("Base angle: " << baseAngle << " selectedIndex: " << menuSelectedIndex << " rotaryAccumulator: " << rotaryAccumulator);    

    for (int i = 0; i < count; ++i)
    {
        double angleDeg = baseAngle + i * angleStep;
        double ang = angleDeg * M_PI / 180.0;

        int ix = static_cast<int>(std::round(centerX + radius * std::cos(ang)));
        int iy = static_cast<int>(std::round(centerY + radius * std::sin(ang)));

        bool selected = (i == menuSelectedIndex);

        int iconSize = selected ? 60 : 40;

        lv_obj_t * icon = lv_obj_create(lv_scr_act());
        lv_obj_set_size(icon, iconSize, iconSize);
        lv_obj_set_style_radius(icon, LV_RADIUS_CIRCLE, 0);

        // pick a color per-item (simple hash)
        int nameSum = 0;
        for (size_t k = 0; k < menuItems[i].name.size(); ++k)
        {
            nameSum += static_cast<unsigned char>(menuItems[i].name[k]);
        }
        uint8_t r = static_cast<uint8_t>((nameSum * 37) % 256);
        uint8_t g = static_cast<uint8_t>((nameSum * 73) % 256);
        uint8_t b = static_cast<uint8_t>((nameSum * 21) % 256);

        lv_obj_set_style_bg_color(icon, lv_color_make(r, g, b), 0);
        lv_obj_set_style_bg_opa(icon, LV_OPA_COVER, 0);
        lv_obj_set_style_border_width(icon, selected ? 4 : 2, 0);
        lv_obj_set_style_border_color(icon, lv_palette_main(LV_PALETTE_GREY), 0);

        // place centered at computed position
        lv_obj_set_pos(icon, ix - iconSize / 2, iy - iconSize / 2);

        // add a small label as placeholder icon (first letter)
        lv_obj_t * lbl = lv_label_create(icon);
        std::string s(1, menuItems[i].name.empty() ? '?' : menuItems[i].name[0]);
        lv_label_set_text(lbl, s.c_str());
        lv_obj_center(lbl);
        lv_obj_set_style_text_color(lbl, lv_color_white(), 0);
        if (selected)
        {
            // make selected label larger
            lv_obj_set_style_text_font(lbl, &lv_font_montserrat_28, 0);
        }
    }
}

void MenuScreen::handle_input_event(const InputDeviceType device_type, const struct input_event &event)
{
    if (device_type == InputDeviceType::ROTARY)
    {
        rotaryAccumulator -= event.value;
        // When at the first item allow only positive accumulator (forward rotation),
        // and when at the last item allow only negative accumulator (backward rotation).
        if (menuSelectedIndex <= 0 && rotaryAccumulator < 0)
        {
            rotaryAccumulator = 0;
        }
        if (menuSelectedIndex >= static_cast<int>(menuItems.size()) - 1 && rotaryAccumulator > 0)
        {
            rotaryAccumulator = 0;
        }
        if (rotaryAccumulator >= 100)
        {
            menuSelectedIndex++;
            if (menuSelectedIndex >= static_cast<int>(menuItems.size()))
            {
                menuSelectedIndex = menuItems.size() - 1;
            }
            rotaryAccumulator = 0;
        }
        else if (rotaryAccumulator <= -100)
        {   
            menuSelectedIndex--;
            if (menuSelectedIndex < 0)
            {
                menuSelectedIndex = 0;
            }
            rotaryAccumulator = 0;
        }

        LOG_DEBUG_STREAM("MenuScreen: Rotary event, new selected index: " << menuSelectedIndex << " accumulator: " << rotaryAccumulator);
    }

    if (device_type == InputDeviceType::BUTTON && event.type == EV_KEY && event.code == 't' && event.value == 1)
    {
        if (beeper_ != nullptr)
        {
            beeper_->play(100);
        }

        if (menuSelectedIndex < 0 || menuSelectedIndex >= static_cast<int>(menuItems.size()))
        {
            return; // Invalid index
        }

        if (menuItems[menuSelectedIndex].name == "Back")
        {
            screenManager_->GoToPreviousScreen();
            menuSelectedIndex = 0; // Reset selection
            return;
        }

        MenuItem &selectedItem = menuItems[menuSelectedIndex];

        if (screenManager_ == nullptr)
        {
            LOG_ERROR_STREAM("MenuScreen: screenManager_ is null!");
            return;
        }

        screenManager_->GoToNextScreen(selectedItem.nextScreenId);
    }
}