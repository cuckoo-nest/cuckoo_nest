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
        selectdMenu = menuItems[menuSelectedIndex].GetName();
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

        // Extracted icon creation
        lv_obj_t* created = CreateIcon(i, ix, iy, selected, iconSize);
        (void)created; // created is parented to screen by CreateIcon
    }
}

lv_obj_t* MenuScreen::CreateIcon(int index, int ix, int iy, bool selected, int iconSize)
{
    lv_obj_t * icon = lv_obj_create(lv_scr_act());
    lv_obj_set_size(icon, iconSize, iconSize);
    lv_obj_set_style_radius(icon, LV_RADIUS_CIRCLE, 0);
    // Disable scrolling/scrollbars on the icon to avoid scroll artifacts
    lv_obj_clear_flag(icon, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(icon, LV_SCROLLBAR_MODE_OFF);

    // pick a color per-item (simple hash)
    int nameSum = 0;
    for (size_t k = 0; k < menuItems[index].GetName().size(); ++k)
    {
        nameSum += static_cast<unsigned char>(menuItems[index].GetName()[k]);
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

    // LVGL symbol list to choose from for demo (fallback will be letter)
    static const char* symbols[] = {
        LV_SYMBOL_OK,
        LV_SYMBOL_CLOSE,
        LV_SYMBOL_HOME,
        LV_SYMBOL_POWER,
        LV_SYMBOL_SETTINGS,
        LV_SYMBOL_GPS,
        LV_SYMBOL_BLUETOOTH,
        LV_SYMBOL_WIFI,
        LV_SYMBOL_USB,
        LV_SYMBOL_BELL,
        LV_SYMBOL_WARNING,
        LV_SYMBOL_TRASH
    };
    const size_t symCount = sizeof(symbols) / sizeof(symbols[0]);

    // choose a symbol index pseudo-randomly from the name hash for demo
    size_t symIndex = static_cast<size_t>(nameSum) % symCount;

    // add a small label for the symbol or fallback to first letter
    lv_obj_t * lbl = lv_label_create(icon);
    const std::string &name = menuItems[index].GetName();
    if (!name.empty())
    {
        // 50% chance to use symbol demo vs fallback first letter
        //if ((nameSum % 2) == 0)
        if(true)
        {
            lv_label_set_text(lbl, symbols[symIndex]);
        }
        else
        {
            std::string s(1, name[0]);
            lv_label_set_text(lbl, s.c_str());
        }
    }
    else
    {
        lv_label_set_text(lbl, "?");
    }
    lv_obj_center(lbl);
    lv_obj_set_style_text_color(lbl, lv_color_white(), 0);
    if (selected)
    {
        lv_obj_set_style_text_font(lbl, &lv_font_montserrat_28, 0);
    }
    // Disable scrolling/scrollbars on the label as well
    lv_obj_clear_flag(lbl, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(lbl, LV_SCROLLBAR_MODE_OFF);

    return icon;
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

        if (menuItems[menuSelectedIndex].GetName() == "Back")
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

        screenManager_->GoToNextScreen(selectedItem.GetNextScreenId());
    }
}