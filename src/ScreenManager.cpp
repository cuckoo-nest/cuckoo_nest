#include <fstream>
#include <sstream>
#include <json11.hpp>
#include <algorithm>
#include <stdio.h>

#include "logger.h"
#include "ScreenManager.hpp"
#include "HAL/HAL.hpp"

#include "Screens/HomeScreen.hpp"
#include "Screens/MenuScreen.hpp"
#include "Screens/SwitchScreen.hpp"
#include "Screens/DimmerScreen.hpp"
#include "Screens/AnalogClockScreen.hpp"

void ScreenManager::GoToNextScreen(std::string const & id)
{
    ScreenBase* screen = (id != "" ? GetScreenById(id) : nullptr);
    if (screen == nullptr)
    {
        LOG_ERROR_STREAM("ScreenManager: Unable to find screen \"" << id << "\"");
        return;
    }

    if(current_screen_)
        current_screen_->OnChangeFocus(false);
    current_screen_ = screen;
    screen_history_.push(current_screen_);
    current_screen_->OnChangeFocus(true);
    LOG_INFO_STREAM("ScreenManager: Navigated to screen ID \"" << id << "\" / \"" << current_screen_->GetName() << "\"");
}

void ScreenManager::GoToPreviousScreen()
{
    if(screen_history_.size() > 1)
    {
        if(current_screen_)
            current_screen_->OnChangeFocus(false);
        screen_history_.pop();
        current_screen_ = screen_history_.top();
        if(current_screen_)
        {
            current_screen_->OnChangeFocus(true);
            LOG_INFO_STREAM("ScreenManager: Navigated to screen ID \"" << current_screen_->GetId() << "\" / \"" << current_screen_->GetName() << "\"");
        }
    }
}

void ScreenManager::ProcessInputEvent(const InputDeviceType device_type, const struct input_event &event)
{
    if (current_screen_ != nullptr)
        current_screen_->handle_input_event(device_type, event);
}

void ScreenManager::LoadScreensFromConfig(const std::string& config_path)
{
    auto configContent = ReadFileContents(config_path);
    if (configContent.empty())
        // Handle error: could not read config file
        return;

    std::string parse_error;
    json11::Json parsed_json = json11::Json::parse(configContent, parse_error, json11::JsonParse::COMMENTS);

    if (!parse_error.empty())
    {
        LOG_ERROR_STREAM("ScreenManager: JSON parse error: " << parse_error);
        return;
    }
    
    if (!parsed_json.is_object())
    {
        LOG_ERROR_STREAM("ScreenManager: Root JSON element must be an object");
        return;
    }

    for (const auto& screen : parsed_json["screens"].array_items())
    {
        std::string name = (!screen["name"].is_null() ? screen["name"].string_value() : "");
        std::string id = (!screen["id"].is_null() ? screen["id"].string_value() : name);
        std::string type = (!screen["type"].is_null() ? screen["type"].string_value() : "");
        transform(type.begin(), type.end(), type.begin(), ::tolower);

        if (type == "home") 
            AddScreen(std::unique_ptr<ScreenBase>(new HomeScreen(this, screen)));
        else if (type == "menu") 
            BuildMenuScreenFromJSON(screen);
        else if (type == "switch") 
            AddScreen(std::unique_ptr<ScreenBase>(new SwitchScreen(this, screen)));
        else if (type == "dimmer") 
            AddScreen(std::unique_ptr<ScreenBase>(new DimmerScreen(this, screen)));
        else if (type == "analogclock") 
            AddScreen(std::unique_ptr<ScreenBase>(new AnalogClockScreen(this, screen)));
        else 
            LOG_ERROR_STREAM(
                "ScreenManager: Unknown screen type '" << type
                << "' for screen \"" << id << "\" / " << "\"" << name << "\""
            );
    }
}

std::string ScreenManager::ReadFileContents(const std::string& filepath) const
{
    std::ifstream file(filepath);
    if (!file.is_open())
        return "";
    
    std::stringstream buffer;
    buffer << file.rdbuf();

    return buffer.str();
}

void ScreenManager::BuildMenuScreenFromJSON(const json11::Json &screenJson)
{
    std::map<std::string, MenuIcon> menuIcons =
    {
        {"", MenuIcon::NONE},
        {"none", MenuIcon::NONE},
        {"ok", MenuIcon::OK},
        {"close", MenuIcon::CLOSE},
        {"home", MenuIcon::HOME},
        {"power", MenuIcon::POWER},
        {"settings", MenuIcon::SETTINGS},
        {"gps", MenuIcon::GPS},
        {"wifi", MenuIcon::WIFI},
        {"usb", MenuIcon::USB},
        {"bell", MenuIcon::BELL},
        {"trash", MenuIcon::TRASH},
        {"breifcase", MenuIcon::BREIFCASE},
        {"light", MenuIcon::LIGHT},
        {"fan", MenuIcon::FAN},
        {"temperature", MenuIcon::TEMPERATURE},
        // {"", MenuIcon::},
        {"stop", MenuIcon::STOP},
        {"left", MenuIcon::LEFT},
        {"right", MenuIcon::RIGHT},
        {"plus", MenuIcon::PLUS},
        {"warning", MenuIcon::WARNING},
        {"up", MenuIcon::UP},
        {"down", MenuIcon::DOWN},
    };

    auto screen = new MenuScreen(this, screenJson);

    // if there is a "nextScreen", add a "Previous"
    std::string screenNext = screen->GetNextScreenId();
    if(screenNext != "")
        screen->AddMenuItem(MenuItem("Previous", "", menuIcons["left"], true));

    // add menu entries as configured
    for (const auto& itemJson : screenJson["menuItems"].array_items()) 
    {
        std::string itemIconStr = itemJson["icon"].string_value();
        transform(itemIconStr.begin(), itemIconStr.end(), itemIconStr.begin(), ::tolower);
        auto menuIconsIt = menuIcons.find(itemIconStr);

        std::string nextScreen = (
            itemJson["nextScreen"].is_number()
            ? std::to_string(itemJson["nextScreen"].int_value())
            : 
                (
                    itemJson["nextScreen"].is_string()
                    ? itemJson["nextScreen"].string_value()
                    : ""
                )
            );

        screen->AddMenuItem(MenuItem(
            itemJson["name"].string_value()
            , nextScreen
            , menuIconsIt != menuIcons.end() ? menuIconsIt->second : menuIcons[""]
        ));
    }

    // configure a "Next" or "Back" final menu item
    MenuIcon menuLastIcon = (screenNext == "" ? menuIcons["close"] : menuIcons["right"]);
    std::string menuLastName= (screenNext == "" ? "Back" : "Next");
    bool isPrevious = (screenNext == "");
    screen->AddMenuItem(MenuItem(menuLastName, screenNext, menuLastIcon, isPrevious));

    // in conclusion
    AddScreen(std::unique_ptr<ScreenBase>(screen));
}
