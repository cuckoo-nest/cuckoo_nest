#pragma once

#include "ScreenBase.hpp"
#include <lvgl/lvgl.h>
#include "../ScreenManager.hpp"
#include "../HAL/IDisplay.hpp"
#include "../HAL/Beeper.hpp"
#include "../Integrations/IntegrationActionBase.hpp"
#include "MenuIcon.hpp"
#include "MenuItem.hpp"


class MenuScreen : public ScreenBase
{
public:
    MenuScreen(HAL *hal,
        ScreenManager* screenManager) : 
        ScreenBase(), 
        screenManager_(screenManager),
        display_(hal->display),
        beeper_(hal->beeper),
        menuSelectedIndex(0),
        rotaryAccumulator(0) {}

    virtual ~MenuScreen() = default;

    void Render() override;
    void handle_input_event(const InputDeviceType device_type, const struct input_event &event) override;
    void AddMenuItem(const MenuItem& item) {
        menuItems.push_back(item);
    }
    int CountMenuItems() const {
        return menuItems.size();
    }

    // Create icon helper: creates and returns an lvgl object for the menu item
    lv_obj_t* CreateIcon(int index, int ix, int iy, bool selected, int iconSize);

private:
    ScreenManager* screenManager_;
    Beeper* beeper_;
    IDisplay* display_;
    int menuSelectedIndex;
    int rotaryAccumulator;
    std::vector<MenuItem> menuItems;

    static constexpr int RotaryAccumulatorThreshold = 500;
};