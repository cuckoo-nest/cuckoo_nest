#pragma once

#include "IntegrationDimmerBase.hpp"
#include "HomeAssistantCreds.hpp"
#include"CurlWrapper.hpp"

class HomeAssistantDimmer : public IntegrationDimmerBase
{
public:
    HomeAssistantDimmer(const HomeAssistantCreds &creds, const std::string &entity_id) : creds_(creds),
                                                                                         entityId_(entity_id), brightness_(-1)
    {
    }

    virtual ~HomeAssistantDimmer() = default;

    const std::string &GetEntityId() const { return entityId_; }

    SwitchState GetState() override
    {
        // Implementation to get state from Home Assistant
        return brightness_ > 0 ? SwitchState::ON : SwitchState::OFF; // Placeholder
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
    int GetBrightness() override {  // <-- THIS IS REQUIRED
        return brightness_;
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
        spdlog::info("Calling Home Assistant service: {}", creds_.GetUrl());
        spdlog::info("Bearer token: {}", creds_.GetToken());

        static CurlWrapper curl_wrapper;
        static bool curl_initialized = false;

        if (!curl_initialized)
        {
            curl_initialized = curl_wrapper.initialize();
            if (!curl_initialized)
            {
                spdlog::error("Failed to initialize libcurl - functionality disabled");
                return;
            }
        }

        // Prepare JSON data
        std::string jsonData = "{\"entity_id\": \"" + entityId_ + "\"}";
        if (brightness >= 0)
        {
            int ha_brightness = (brightness * 255) / 100;
            jsonData = "{\"entity_id\": \"" + entityId_ + "\", \"brightness\": " + std::to_string(ha_brightness) + "}";
        }

        // Prepare headers
        struct curl_slist *headers = nullptr;
        std::string authHeader = "Authorization: Bearer " + creds_.GetToken();
        headers = curl_wrapper.slist_append(headers, authHeader.c_str());
        headers = curl_wrapper.slist_append(headers, "Content-Type: application/json");

        CURL *curl = curl_wrapper.easy_init();
        if (curl)
        {
            std::string url = creds_.GetUrl() + "/api/services/" + action;
            spdlog::info("HomeAssistantDimmer: URL: {}", url);
            curl_wrapper.easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_wrapper.easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
            curl_wrapper.easy_setopt(curl, CURLOPT_POSTFIELDS, jsonData.c_str());

            CURLcode res = curl_wrapper.easy_perform(curl);
            if (res != CURLE_OK)
            {
                spdlog::error("curl_easy_perform() failed: {}", curl_wrapper.easy_strerror(res));
            }

            curl_wrapper.easy_cleanup(curl);
            curl_wrapper.slist_free_all(headers);
            spdlog::info("HomeAssistantDimmer: Service call completed");
        }
    }
};
