#pragma once

#include <memory>
#include <vector>
#include <stack>
#include <map>
#include <cstddef>
#include <json11.hpp>
#include "HAL/HAL.hpp"
#include "Screens/ScreenBase.hpp"
#include "Screens/MenuIcon.hpp"
#include "Integrations/IntegrationContainer.hpp"
#include "Backplate/BackplateComms.hpp"

class ScreenManager 
{
public:
    ScreenManager(
        HAL *hal, 
        IntegrationContainer* integrationContainer,
        BackplateComms *backplateComms);
    ~ScreenManager();
    
    void GoToNextScreen(int id);
    void GoToPreviousScreen();
    void RenderCurrentScreen();
    void ProcessInputEvent(const InputDeviceType device_type, const input_event &event);

    void LoadScreensFromConfig(const std::string &config_path);

    size_t CountScreens() const { return screens_.size(); }
    
    ScreenBase* GetScreenById(int id) const {
        auto it = screens_.find(id);
        return (it != screens_.end()) ? it->second.get() : nullptr;
    }

    void AddScreen(std::unique_ptr<ScreenBase> screen) {
        int id = screen->GetId();
        screens_[id] = std::move(screen);
    }

    IntegrationContainer* GetIntegrationContainer() const {
        return integrationContainer_;
    }

private:
    std::string ReadFileContents(const std::string &filepath) const;

    void BuildHomeScreenFromJSON(const json11::Json &screenJson, int id);
    void BuildMenuScreenFromJSON(const json11::Json &screenJson, int id);
    MenuIcon DetermineMenuIcon(std::string &iconStr);
    void BuildSwitchScreenFromJSON(const json11::Json &screenJson, int id);
    void BuildDimmerScreenFromJSON(const json11::Json &screenJson, int id);
    void BuildAnalogClockScreenFromJson(const json11::Json &screenJson, int id);

    std::stack<ScreenBase*> screen_history_;
    ScreenBase* current_screen_;
    std::map<int, std::unique_ptr<ScreenBase>> screens_;
    HAL *hal_;
    IntegrationContainer* integrationContainer_;
    BackplateComms *backplateComms_;
};