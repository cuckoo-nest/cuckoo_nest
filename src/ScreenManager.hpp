#pragma once

#include <memory>
#include <vector>
#include <stack>
#include <map>
#include <cstddef>
#include <json11.hpp>

#include "HAL/HAL.hpp"
#include "Screens/ScreenBase.hpp"
#include "Integrations/IntegrationContainer.hpp"
#include "Backplate/BackplateComms.hpp"

class ScreenManager 
{
public:
    ScreenManager(
        HAL *hal
        , IntegrationContainer* integrationContainer
        , BackplateComms *backplateComms
    )
        : hal_(hal)
        , integrationContainer_(integrationContainer)
        , backplateComms_(backplateComms)
    {}

    virtual ~ScreenManager() {};
    
    void GoToNextScreen(std::string const &id);
    void GoToPreviousScreen();
    void ProcessInputEvent(const InputDeviceType device_type, const input_event &event);

    void LoadScreensFromConfig(const std::string &config_path);
    inline size_t CountScreens() const { return screens_.size(); } // for test harness

    ScreenBase* GetScreenById(std::string const &id) const
    {
        auto it = screens_.find(id);
        return (it != screens_.end()) ? it->second.get() : nullptr;
    }
    inline void AddScreen(std::unique_ptr<ScreenBase> screen) { screens_[screen->GetId()] = std::move(screen); }

    inline HAL *Hal() const { return hal_; }
    inline IDisplay *HalDisplay() const { return Hal() != nullptr ? Hal()->display : nullptr; }
    inline Beeper *HalBeeper() const { return Hal() != nullptr ? Hal()->beeper : nullptr; }
    inline Inputs *HalInputs() const { return Hal() != nullptr ? Hal()->inputs : nullptr; }
    inline Backlight *HalBacklight() const { return Hal() != nullptr ? Hal()->backlight : nullptr; }

    inline IntegrationContainer* GetIntegrationContainer() const { return integrationContainer_; }
    inline BackplateComms *GetBackplaceComms() const { return backplateComms_; }

private:
    std::string ReadFileContents(const std::string &filepath) const;

    void BuildMenuScreenFromJSON(const json11::Json &screenJson);

    std::stack<ScreenBase*> screen_history_;
    ScreenBase* current_screen_ = nullptr;
    std::map<std::string, std::unique_ptr<ScreenBase>> screens_;

    HAL *hal_ = nullptr;
    IntegrationContainer* integrationContainer_ = nullptr;
    BackplateComms *backplateComms_ = nullptr;
};