#pragma once

#include <cmath>

#include "IntegrationDimmerBase.hpp"
#include "HomeAssistantCreds.hpp"
#include "logger.h"

class HomeAssistantDimmer : public HomeAssistantBase, public IntegrationDimmerBase
{
public:
    HomeAssistantDimmer(const HomeAssistantCreds &creds, const std::string &entity_id)
        : IntegrationDimmerBase()
        , HomeAssistantBase()
        , creds_(creds)
        , entityId_(entity_id)
        , brightness_(-1)
    {
    }

    virtual ~HomeAssistantDimmer() = default;

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
                    && brightness_ > 0
                ? SwitchState::ON
                : SwitchState::OFF
            );
        return ss;
    }

    void TurnOn() override
    {
        // Implementation to turn on the Dimmer via Home Assistant
        SetBrightness(100);
    }

    void TurnOff() override
    {
        // Implementation to turn off the Dimmer via Home Assistant
        SetBrightness(-1);
    }
    int GetBrightness() override
    {
        json11::Json status = queryStatus(GetEntityId(), creds_);
        int brightness = 128;
        if(status.is_object() && !status["attributes"].is_null())
        {
            json11::Json attribs = status["attributes"];
            brightness = (
                !attribs["brightness"].is_null()
                && attribs["brightness"].is_number()
                ? attribs["brightness"].int_value()
                : 0
                );
        }
        LOG_INFO_STREAM("Dimmer::GetBrightness brightness value = " << brightness);
        if(brightness > 0)
        {
            #ifdef BUILD_TARGET_NEST
            // Because float/double is broken.
            // TODO - remove special case when -mfloat-abi=soft is removed/fixed
            brightness *= 100;
            brightness /= 255;
            brightness += 1; // all the integer math forces round-down, so we... bump it by one
            #else
            double b = brightness / 2.55;
            brightness = static_cast<int>(std::round(b));
            #endif
        }
        LOG_INFO_STREAM("Dimmer::GetBrightness brightness = " << brightness << "%");
        if (brightness > 100) brightness = 100;

        return brightness;
    }

    void SetBrightness(int brightness) override
    {
        if (brightness <= 0)
        {
            brightness_ = -1;                  
            ExecuteBrightness("light/turn_off");
        }
        else
        {
            if (brightness > 100) brightness = 100;
            brightness_ = brightness;
            ExecuteBrightness("light/turn_on", brightness_);
        }
    }

private:
    HomeAssistantCreds creds_;
    std::string entityId_;
    int brightness_;

    void ExecuteBrightness(std::string action, int brightness = -1)
    {
        // Implementation to call Home Assistant service
        LOG_INFO_STREAM("Calling Home Assistant service: " << creds_.GetUrl());

        // Prepare JSON data
        std::string jsonData = "{\"entity_id\": \"" + entityId_ + "\"}";
        if (brightness >= 0)
        {
            int ha_brightness = (brightness * 255) / 100;
            jsonData = "{\"entity_id\": \"" + entityId_ + "\", \"brightness\": " + std::to_string(ha_brightness) + "}";
        }
        queryExecute(action, jsonData, creds_);
    }
};
