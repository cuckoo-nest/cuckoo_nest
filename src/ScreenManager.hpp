#pragma once

#include <memory>
#include <vector>
#include "Screens/ScreenBase.hpp"

class ScreenManager 
{
public:
    ScreenManager();
    ~ScreenManager();
    
    void GoToNextScreen(ScreenBase* screen);
    void GoToPreviousScreen();


private:
    ScreenBase* current_screen_;
    ScreenBase* previous_screen_;
};