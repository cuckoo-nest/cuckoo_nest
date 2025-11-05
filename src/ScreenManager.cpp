#include "ScreenManager.hpp"

ScreenManager::ScreenManager(): 
    screen_history_(),
    current_screen_(nullptr)
{
}

ScreenManager::~ScreenManager()
{
}

void ScreenManager::GoToNextScreen(ScreenBase *screen)
{
    screen_history_.push(current_screen_);
    current_screen_ = screen;
    current_screen_->Render();
}

void ScreenManager::GoToPreviousScreen()
{
    if (!screen_history_.empty()) {
        // if (current_screen_ != nullptr)
        // {
        //     delete current_screen_;
        // }

        current_screen_ = screen_history_.top();
        screen_history_.pop();
        current_screen_->Render();
    }
}

void ScreenManager::RenderCurrentScreen()
{
    if (current_screen_ != nullptr)
    {
        current_screen_->Render();
    }
}

void ScreenManager::ProcessInputEvent(const InputDeviceType device_type, const struct input_event &event)
{
    if (current_screen_ != nullptr)
    {
        current_screen_->handle_input_event(device_type, event);
    }
}