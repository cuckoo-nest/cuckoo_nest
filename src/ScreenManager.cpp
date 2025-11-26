#include "ScreenManager.hpp"
#include <fstream>
#include <sstream>
#include <json11.hpp>
#include <spdlog/spdlog.h>
#include <algorithm>
#include <stdio.h>

#include "HAL/HAL.hpp"

#include "Screens/HomeScreen.hpp"
#include "Screens/MenuScreen.hpp"
#include "Screens/SwitchScreen.hpp"
#include "Screens/DimmerScreen.hpp"

ScreenManager::ScreenManager(HAL *hal, IntegrationContainer* integrationContainer) : 
    screen_history_(),
    current_screen_(nullptr),
    hal_(hal),
    integrationContainer_(integrationContainer)
{
}

ScreenManager::~ScreenManager()
{
}

void ScreenManager::GoToNextScreen(int id)
{
    ScreenBase* screen = GetScreenById(id);
    if (screen == nullptr)
    {
        return;
    }
    current_screen_ = screen;
    screen_history_.push(current_screen_);
    current_screen_->Render();
    spdlog::info("ScreenManager: Navigated to screen ID {}", id);
}

void ScreenManager::GoToPreviousScreen()
{
    if (!screen_history_.empty()) {
        screen_history_.pop();
        current_screen_ = screen_history_.top();
        current_screen_->Render();
    }
}

void ScreenManager::RenderCurrentScreen()
{
    if (current_screen_ != nullptr)
    {
        current_screen_->Render();
    }
    else
    {
        spdlog::error("ScreenManager: No current screen to render");
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
        spdlog::error("ScreenManager: JSON parse error: {}", parse_error);
        return;
    }
    
    if (!parsed_json.is_object()) {
        spdlog::error("ScreenManager: Root JSON element must be an object");
        return;
    }

    for (const auto& screen : parsed_json["screens"].array_items()) 
    {
        if (!screen.is_object()) {
            continue; // skip invalid entries
        }

        int id = screen["id"].int_value();
        if (id == 0) {
            continue; // skip invalid or missing IDs
        }

        std::string name = screen["name"].string_value();
        std::string type = screen["type"].string_value();
        transform(type.begin(), type.end(), type.begin(), ::tolower);

        if (type == "home") 
        {
            BuildHomeScreenFromJSON(screen, id);
        }
        else if (type == "menu") 
        {
            BuildMenuScreenFromJSON(screen, id);
        }
        else if (type == "switch") 
        {
            BuildSwitchScreenFromJSON(screen, id);
        }
        else if (type == "dimmer") 
        {
            BuildDimmerScreenFromJSON(screen, id);
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

void ScreenManager::BuildHomeScreenFromJSON(const json11::Json &screenJson, int id)
{
    int nextScreenId = screenJson["nextScreen"].int_value();
    auto homeScreen = new HomeScreen(hal_, this);
    homeScreen->SetNextScreenId(nextScreenId);
    screens_[id] = std::unique_ptr<ScreenBase>(homeScreen);
}

void ScreenManager::BuildMenuScreenFromJSON(const json11::Json &screenJson, int id)
{
    auto menuScreen = new MenuScreen(hal_, this);
    
    for (const auto& itemJson : screenJson["menuItems"].array_items()) 
    {
        std::string itemName = itemJson["name"].string_value();
        int nextScreenId = itemJson["nextScreen"].int_value();
        menuScreen->AddMenuItem(MenuItem(itemName, nextScreenId));
    }

    menuScreen->AddMenuItem(MenuItem("Back", -1)); // Add Back option
    
    screens_[id] = std::unique_ptr<ScreenBase>(menuScreen);
}

void ScreenManager::BuildSwitchScreenFromJSON(const json11::Json &screenJson, int id)
{
    int integrationId = screenJson["integrationId"].int_value();
    std::string name = screenJson["name"].string_value();
    auto switchScreen = new SwitchScreen(hal_, this);
    switchScreen->SetIntegrationId(integrationId);
    switchScreen->SetName(name);
    screens_[id] = std::unique_ptr<ScreenBase>(switchScreen);
}

void ScreenManager::BuildDimmerScreenFromJSON(const json11::Json &screenJson, int id)
{
    int integrationId = screenJson["integrationId"].int_value();
    auto dimmerScreen = new DimmerScreen(hal_, this);
    dimmerScreen->SetIntegrationId(integrationId);
    screens_[id] = std::unique_ptr<ScreenBase>(dimmerScreen);
}
