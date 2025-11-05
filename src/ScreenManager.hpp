#pragma once

#include <memory>
#include <vector>
#include <stack>
#include "Screens/ScreenBase.hpp"

class ScreenManager 
{
public:
    ScreenManager();
    ~ScreenManager();
    
    void GoToNextScreen(ScreenBase* screen);
    void GoToPreviousScreen();
    void RenderCurrentScreen();
    void ProcessInputEvent(const InputDeviceType device_type, const input_event &event);

private:
    std::stack<ScreenBase*> screen_history_;
    ScreenBase* current_screen_;
};