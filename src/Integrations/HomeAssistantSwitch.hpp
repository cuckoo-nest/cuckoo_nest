#pragma once

#include "HomeAssistantBase.hpp"
#include "HomeAssistantCreds.hpp"
#include "CurlWrapper.hpp"
#include "logger.h"
#include <json11.hpp>

class HomeAssistantSwitch : public HomeAssistantBase, public IntegrationSwitchBase
{
public:
    HomeAssistantSwitch(const HomeAssistantCreds &creds, const std::string &entity_id)
        : IntegrationSwitchBase()
        , HomeAssistantBase()
        , creds_(creds)
        , entityId_(entity_id)
    {}

    virtual ~HomeAssistantSwitch() = default;

    const std::string &GetEntityId() const { return entityId_; }

    SwitchState GetState() override
    {
        // Implementation to get state from Home Assistant
        json11::Json status = queryStatus(GetEntityId(), creds_);
        SwitchState ss =
            (
                status.is_object()
                    && !status["state"].is_null()
                    && status["state"].is_string()
                    && status["state"].string_value() == "on"
                ? SwitchState::ON
                : SwitchState::OFF
            );

        return ss;
    }

    void TurnOn() override
    {
        // Implementation to turn on the switch via Home Assistant
        execute("switch/turn_on");
    }

    void TurnOff() override
    {
        // Implementation to turn off the switch via Home Assistant
        execute("switch/turn_off");
    }

private:
    HomeAssistantCreds creds_;
    std::string entityId_;

    void execute(std::string action)
    {
        // Implementation to call Home Assistant service
        LOG_INFO_STREAM("Calling Home Assistant service: " << creds_.GetUrl());

        // Prepare JSON data
        std::string jsonData = "{\"entity_id\": \"" + entityId_ + "\"}";
        queryExecute(action, jsonData, creds_);
    }
};