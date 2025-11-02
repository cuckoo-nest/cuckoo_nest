#include "ScreenManager.hpp"

ScreenManager::ScreenManager()
    : current_screen_(nullptr), 
    previous_screen_(nullptr)
{
}

ScreenManager::~ScreenManager()
{
}

void ScreenManager::GoToNextScreen(ScreenBase *screen)
{
    previous_screen_ = current_screen_;
    current_screen_ = screen;
}

void ScreenManager::GoToPreviousScreen()
{
    if (previous_screen_ != nullptr) {
        current_screen_ = previous_screen_;
        previous_screen_ = nullptr;
    }
}