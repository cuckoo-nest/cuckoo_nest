#include "MenuScreen.hpp"
#include <spdlog/spdlog.h>

void MenuScreen::Render()
{
    if (display_ == nullptr)
    {
        return;
    }

    
    display_->SetBackgroundColor(SCREEN_COLOR_BLACK);

    display_->DrawText(60, -100, "MenuName", SCREEN_COLOR_WHITE, Font::FONT_H1);
    
    int idx = 0;
    for (const auto &item : menuItems)
    {
        std::string menuName = item.name;
        if (idx == menuSelectedIndex)
        {
            menuName = "> " + menuName; // Indicate selection
        }
        
        int yOffset = (idx - menuSelectedIndex) * 25;
        if (idx < menuSelectedIndex - 3 || idx > menuSelectedIndex + 3)
        {
            yOffset += 200; // Move off-screen
        }
        
        display_->DrawText(60, yOffset, menuName, SCREEN_COLOR_WHITE, Font::FONT_H2);
        idx++;
    }
}

void MenuScreen::handle_input_event(const InputDeviceType device_type, const struct input_event &event)
{
    if (device_type == InputDeviceType::ROTARY)
    {
        rotaryAccumulator -= event.value;
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

        spdlog::debug("MenuScreen: Rotary event, new selected index: {} accumulator: {}", 
                      menuSelectedIndex, rotaryAccumulator);
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
            spdlog::error("MenuScreen: screenManager_ is null!");
            return;
        }

        screenManager_->GoToNextScreen(selectedItem.nextScreenId);
    }
}