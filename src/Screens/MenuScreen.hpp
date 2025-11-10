#pragma once

#include "ScreenBase.hpp"
#include "../ScreenManager.hpp"
#include "../HAL/Display.hpp"
#include "../HAL/Beeper.hpp"
#include "../Integrations/IntegrationActionBase.hpp"

class MenuItem
{
    public:
        MenuItem(const std::string& name, int nextScreenId) : 
            name(name), 
            nextScreenId(nextScreenId) {}

        std::string name;
        int nextScreenId;
};

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

private:
    ScreenManager* screenManager_;
    Beeper* beeper_;
    Display* display_;
    int menuSelectedIndex;
    int rotaryAccumulator;
    std::vector<MenuItem> menuItems;
};