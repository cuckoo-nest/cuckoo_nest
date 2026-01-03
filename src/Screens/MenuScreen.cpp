#include "MenuScreen.hpp"
#include "logger.h"
#include <lvgl/lvgl.h>
#include <cmath>
#include "../fonts/CuckooFontAwesomeDefs.h"

extern "C" {
    LV_FONT_DECLARE(CuckooFontAwesome);
}

void MenuScreen::Render()
{
    if (display_ == nullptr)
        return;

    display_->SetBackgroundColor(SCREEN_COLOR_BLACK);
    
    std::string selectdMenu = "No ITems";
    if (!menuItems.empty())
        selectdMenu = menuItems[menuSelectedIndex].GetName();
    display_->DrawText(0, 0, selectdMenu, SCREEN_COLOR_WHITE, Font::FONT_H2);
        
    // Draw circular icons representing each menu item using LVGL directly.
    // Icons are placed around a circle; the selected item is rotated to the top.
    const int screenW = display_->width();
    const int screenH = display_->height();
    const int centerX = screenW / 2;
    const int centerY = screenH / 2;
    const int radius = 120;

    int count = static_cast<int>(menuItems.size());
    if (count <= 0)
        return;

    const double angleStep = 360.0 / count;
    // rotate so the selected index is at -90 degrees (top)
    // incorporate rotaryAccumulator for fractional rotation between items
    double frac = 0.0;
    if (std::abs(rotaryAccumulator) > 10)
    {
        frac = (double)rotaryAccumulator / RotaryAccumulatorThreshold;
        if (frac > 0.99) frac = 0.99;
        if (frac < -0.99) frac = -0.99;
    }

    // Prevent rotation beyond list bounds: when at first or last item, disallow
    // fractional movement that would visually spin icons past the ends.
    if (menuSelectedIndex <= 0 && frac < 0.0)
        frac = 0.0;
    if (menuSelectedIndex >= count - 1 && frac > 0.0)
        frac = 0.0;
    
    const double baseAngle = -90.0 - ((double)menuSelectedIndex + frac) * angleStep;

    for (int i = 0; i < count; ++i)
    {
        double angleDeg = baseAngle + i * angleStep;
        int16_t angle = static_cast<int16_t>(angleDeg);

        // Use LVGL's integer trigonometry (works around GCC 4.9.4 soft-float math lib bug)
        // lv_trigo_sin/cos return fixed-point: -32768 to 32768 representing -1.0 to 1.0
        // Optimized: (radius * trig_value) >> 15 instead of radius * (trig_value / 32768.0)
        int ix = centerX + ((radius * lv_trigo_cos(angle)) >> 15);
        int iy = centerY + ((radius * lv_trigo_sin(angle)) >> 15);

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
        nameSum += static_cast<unsigned char>(menuItems[index].GetName()[k]);

    uint8_t r = static_cast<uint8_t>((nameSum * 37) % 256);
    uint8_t g = static_cast<uint8_t>((nameSum * 73) % 256);
    uint8_t b = static_cast<uint8_t>((nameSum * 21) % 256);

    lv_obj_set_style_bg_color(icon, lv_color_make(r, g, b), 0);
    lv_obj_set_style_bg_opa(icon, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(icon, selected ? 4 : 2, 0);
    lv_obj_set_style_border_color(icon, lv_palette_main(LV_PALETTE_GREY), 0);

    // place centered at computed position
    lv_obj_set_pos(icon, ix - iconSize / 2, iy - iconSize / 2);

    // if no icon specified/found, we will use first letter of name instead
    const char* symbol = nullptr;
    MenuIcon menuIcon = menuItems[index].GetIcon();
    std::map<MenuIcon, const char *> mapMenuIconSymbol =
    {
        {MenuIcon::NONE, nullptr},
        {MenuIcon::OK, CUCKOO_SYMBOL_OK},
        {MenuIcon::CLOSE, CUCKOO_SYMBOL_CLOSE},
        {MenuIcon::HOME, CUCKOO_SYMBOL_HOME},
        {MenuIcon::POWER, CUCKOO_SYMBOL_POWER},
        {MenuIcon::SETTINGS, CUCKOO_SYMBOL_SETTINGS},
        {MenuIcon::GPS, CUCKOO_SYMBOL_GPS},
        {MenuIcon::BLUETOOTH, CUCKOO_SYMBOL_BLUETOOTH},
        {MenuIcon::WIFI, CUCKOO_SYMBOL_WIFI},
        {MenuIcon::USB, CUCKOO_SYMBOL_USB},
        {MenuIcon::BELL, CUCKOO_SYMBOL_BELL},
        {MenuIcon::WARNING, CUCKOO_SYMBOL_WARNING},
        {MenuIcon::TRASH, CUCKOO_SYMBOL_TRASH},
        {MenuIcon::BREIFCASE, CUCKOO_SYMBOL_BREIFCASE},
        {MenuIcon::LIGHT, CUCKOO_SYMBOL_LIGHT},
        {MenuIcon::FAN, CUCKOO_SYMBOL_FAN},
        {MenuIcon::TEMPERATURE, CUCKOO_SYMBOL_TEMPERATURE},
        {MenuIcon::STOP, CUCKOO_SYMBOL_STOP},
        {MenuIcon::LEFT, CUCKOO_SYMBOL_LEFT},
        {MenuIcon::RIGHT, CUCKOO_SYMBOL_RIGHT},
        {MenuIcon::PLUS, CUCKOO_SYMBOL_PLUS},
        {MenuIcon::UP, CUCKOO_SYMBOL_UP},
        {MenuIcon::DOWN, CUCKOO_SYMBOL_DOWN},
        // {MenuIcon::, CUCKOO_SYMBOL_},
    };
    auto mapMenuIconSymbolIt = mapMenuIconSymbol.find(menuIcon);
    if(mapMenuIconSymbolIt != mapMenuIconSymbol.end())
        symbol = mapMenuIconSymbolIt->second;

    // add a small label for the symbol or fallback to first letter
    lv_obj_t * lbl = lv_label_create(icon);
    const std::string &name = menuItems[index].GetName();
    if (!name.empty())
    {
        if(menuItems[index].GetIcon() != MenuIcon::NONE)
        {
            lv_obj_set_style_text_font(lbl, &CuckooFontAwesome, 0);
            lv_label_set_text(lbl, symbol);
        }
        else
        {
            std::string s(1, name[0]);
            lv_label_set_text(lbl, s.c_str());
        }
    }
    else
        lv_label_set_text(lbl, "?");
    
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
        // rotaryAccumulator -= (-1 * event.value);
        // When at the first item allow only positive accumulator (forward rotation),
        // and when at the last item allow only negative accumulator (backward rotation).
        if (menuSelectedIndex <= 0 && rotaryAccumulator < 0)
            rotaryAccumulator = 0;
        if (menuSelectedIndex >= static_cast<int>(menuItems.size()) - 1 && rotaryAccumulator > 0)
            rotaryAccumulator = 0;
        if (rotaryAccumulator >= RotaryAccumulatorThreshold)
        {
            menuSelectedIndex++;
            if (menuSelectedIndex >= static_cast<int>(menuItems.size()))
                menuSelectedIndex = menuItems.size() - 1;
            rotaryAccumulator = 0;
        }
        else if (rotaryAccumulator <= -RotaryAccumulatorThreshold)
        {   
            menuSelectedIndex--;
            if (menuSelectedIndex < 0)
                menuSelectedIndex = 0;
            rotaryAccumulator = 0;
        }

        Render();
    }

    if (device_type == InputDeviceType::BUTTON && event.type == EV_KEY && event.code == 't' && event.value == 1)
    {
        if (beeper_ != nullptr)
            beeper_->click();

        if (menuSelectedIndex < 0 || menuSelectedIndex >= static_cast<int>(menuItems.size()))
            return; // Invalid index

        if (screenManager_ == nullptr)
        {
            LOG_ERROR_STREAM("MenuScreen: screenManager is null!");
            return;
        }

        if (menuItems[menuSelectedIndex].IsPrevious())
        {
            screenManager_->GoToPreviousScreen();
            menuSelectedIndex = 0; // Reset selection
            return;
        }

        MenuItem &selectedItem = menuItems[menuSelectedIndex];

		LOG_DEBUG_STREAM("MenuScreen: selecting " << selectedItem.GetNextScreenId() << " / \"" << selectedItem.GetName() << "\"");
        screenManager_->GoToNextScreen(selectedItem.GetNextScreenId()); 
    }
}
