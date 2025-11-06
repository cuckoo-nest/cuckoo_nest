#pragma once

#include "ScreenBase.hpp"
#include "../ScreenManager.hpp"
#include "../HAL/Display.hpp"
#include "../HAL/Beeper.hpp"
#include "../Integrations/IntegrationActionBase.hpp"

class MenuItem
{
    public:
        MenuItem(const std::string& name, ScreenBase* screen, IntegrationActionBase *action) : 
            name(name), 
            nextScreen(screen),
            callback(action) {}
        
        std::string name;
        ScreenBase* nextScreen;
        IntegrationActionBase *callback;
};

class MenuScreen : public ScreenBase
{
public:
    MenuScreen(
        ScreenManager* screenManager,
        Display *display,
        Beeper *beeper) : 
        ScreenBase(), 
        screenManager_(screenManager),
        display_(display), 
        beeper_(beeper),
        menuSelectedIndex(0),
        rotaryAccumulator(0) {}

    virtual ~MenuScreen() = default;

    void Render() override;
    void handle_input_event(const InputDeviceType device_type, const struct input_event &event) override;
    void AddMenuItem(const MenuItem& item) {
        menuItems.push_back(item);
    }

private:
    ScreenManager* screenManager_;
    Beeper* beeper_;
    Display* display_;
    int menuSelectedIndex;
    int rotaryAccumulator;
    std::vector<MenuItem> menuItems;
};