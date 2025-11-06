#include "MenuScreen.hpp"
#include <iostream>

void MenuScreen::Render()
{
    if (display_ == nullptr)
    {
        return;
    }

    display_->SetBackgroundColor(SCREEN_COLOR_BLACK);

    int idx = 0;
    for (const auto &item : menuItems)
    {
        std::string menuName = item.name;
        if (idx == menuSelectedIndex)
        {
            menuName = "> " + menuName; // Indicate selection
        }
        display_->DrawText(60, 80 + (idx * 25), menuName, SCREEN_COLOR_WHITE, 3);
        idx++;
    }

    display_->Flush();
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
        std::cout << "Menu selected index: " << menuSelectedIndex << ", rotaryAccumulator: " << rotaryAccumulator << std::endl;
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

        MenuItem &selectedItem = menuItems[menuSelectedIndex];

        // If there's a callback, invoke it
        if (selectedItem.callback != nullptr)
        {
            selectedItem.callback->execute();
        }
        else
        {
            std::cout << "MenuScreen: No callback for selected item '" << selectedItem.name << "'\n";
        }

        // If there's a next screen, navigate to it
        if (selectedItem.nextScreen != nullptr)
        {
            screenManager_->GoToNextScreen(selectedItem.nextScreen);
        }
    }
}