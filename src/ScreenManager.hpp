#pragma once

#include <memory>
#include <vector>
#include <stack>
#include <map>
#include <cstddef>
#include "Screens/ScreenBase.hpp"
#include "HAL/HAL.hpp"

class ScreenManager 
{
public:
    ScreenManager(HAL *hal);
    ~ScreenManager();
    
    void GoToNextScreen(ScreenBase* screen);
    void GoToPreviousScreen();
    void RenderCurrentScreen();
    void ProcessInputEvent(const InputDeviceType device_type, const input_event &event);

    void LoadScreensFromConfig(const std::string& config_path);
    
    size_t CountScreens() const { return screens_.size(); }
    
private:
    std::string ReadFileContents(const std::string &filepath) const;
    
    std::stack<ScreenBase*> screen_history_;
    ScreenBase* current_screen_;
    std::map<int, std::unique_ptr<ScreenBase>> screens_;
    HAL *hal_;
};