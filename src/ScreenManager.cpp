#include "ScreenManager.hpp"
#include <fstream>
#include <sstream>
#include <json11.hpp>
#include <iostream>
#include <algorithm>
#include <stdio.h>

#include "HAL/HAL.hpp"

#include "Screens/HomeScreen.hpp"
#include "Screens/MenuScreen.hpp"
#include "Screens/SwitchScreen.hpp"
#include "Screens/DimmerScreen.hpp"

ScreenManager::ScreenManager(HAL *hal) : 
    screen_history_(),
    current_screen_(nullptr),
    hal_(hal)
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

void ScreenManager::LoadScreensFromConfig(const std::string& config_path)
{
    auto configContent = ReadFileContents(config_path);
    if (configContent.empty())
    {
        // Handle error: could not read config file
        return;
    }

    std::string parse_error;
    json11::Json parsed_json = json11::Json::parse(configContent, parse_error);

    if (!parse_error.empty()) {
        std::cerr << "ScreenManager: JSON parse error: " << parse_error << std::endl;
        return;
    }
    
    if (!parsed_json.is_object()) {
        std::cerr << "ScreenManager: Root JSON element must be an object" << std::endl;
        return;
    }

    for (const auto& screen : parsed_json["screens"].array_items()) 
    {
        if (!screen.is_object()) {
            continue; // skip invalid entries
        }

        int id = screen["id"].int_value();
        std::string name = screen["name"].string_value();
        std::string type = screen["type"].string_value();
        transform(type.begin(), type.end(), type.begin(), ::tolower);

        if (type == "home") 
        {
            screens_[id] = std::unique_ptr<ScreenBase>(
                new HomeScreen(hal_, this));
        }
        else if (type == "menu") 
        {
            screens_[id] = std::unique_ptr<ScreenBase>(
                new MenuScreen(hal_, this));
        }
        else if (type == "switch") 
        {
            screens_[id] = std::unique_ptr<ScreenBase>(
                new SwitchScreen(hal_, this, nullptr, nullptr));
        }
        else if (type == "dimmer") 
        {
            screens_[id] = std::unique_ptr<ScreenBase>(
                new DimmerScreen(hal_, this));
        }
    }
}

std::string ScreenManager::ReadFileContents(const std::string& filepath) const
{
    std::ifstream file(filepath);
    if (!file.is_open()) {
        return "";
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();

    return buffer.str();
}