#include "MenuScreen.hpp"
#include "logger.h"
#include <lvgl/lvgl.h>
#include <cmath>
#include "../fonts/CuckooFontAwesomeDefs.h"

extern "C" {
    LV_FONT_DECLARE(cuckoo_fontawesome);
}

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

    const char* symbol = nullptr;
    switch (menuItems[index].GetIcon())
    {
        case MenuIcon::OK:
            symbol = CUCKOO_SYMBOL_OK;
            break;
        case MenuIcon::CLOSE:
            symbol = CUCKOO_SYMBOL_CLOSE;
            break;
        case MenuIcon::HOME:
            symbol = CUCKOO_SYMBOL_HOME;
            break;
        case MenuIcon::POWER:
            symbol = CUCKOO_SYMBOL_POWER;
            break;
        case MenuIcon::SETTINGS:
            symbol = CUCKOO_SYMBOL_SETTINGS;
            break;
        case MenuIcon::GPS:
            symbol = CUCKOO_SYMBOL_GPS;
            break; 
        case MenuIcon::BLUETOOTH:
            symbol = CUCKOO_SYMBOL_BLUETOOTH;
            break;
        case MenuIcon::WIFI:
            symbol = CUCKOO_SYMBOL_WIFI;
            break;
        case MenuIcon::USB:
            symbol = CUCKOO_SYMBOL_USB;
            break;
        case MenuIcon::BELL:
            symbol = CUCKOO_SYMBOL_BELL;
            break;
        case MenuIcon::WARNING:
            symbol = CUCKOO_SYMBOL_WARNING;
            break;  
        case MenuIcon::TRASH:
            symbol = CUCKOO_SYMBOL_TRASH;
            break;
        case MenuIcon::BREIFCASE:
            symbol = CUCKOO_SYMBOL_BREIFCASE;
            break;
        case MenuIcon::LIGHT:
            symbol = CUCKOO_SYMBOL_LIGHT;
            break;
        case MenuIcon::FAN:
            symbol = CUCKOO_SYMBOL_FAN;
            break;
        case MenuIcon::TEMPERATURE:
            symbol = CUCKOO_SYMBOL_TEMPERATURE;
            break;

        case MenuIcon::NONE:
        default:
            // No icon specified, we will use first letter of name instead
            break;
    }

    // add a small label for the symbol or fallback to first letter
    lv_obj_t * lbl = lv_label_create(icon);
    const std::string &name = menuItems[index].GetName();
    if (!name.empty())
    {
        if(menuItems[index].GetIcon() != MenuIcon::NONE)
        {
            lv_obj_set_style_text_font(lbl, &cuckoo_fontawesome, 0);
            lv_label_set_text(lbl, symbol);
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