#pragma once

#include "IntegrationSwitchBase.hpp"
#include "HomeAssistantCreds.hpp"
#include"CurlWrapper.hpp"

class HomeAssistantSwitch : public IntegrationSwitchBase
{
public:
    HomeAssistantSwitch(const HomeAssistantCreds &creds, const std::string &entity_id) : creds_(creds),
                                                                                         entityId_(entity_id)
    {
    }

    virtual ~HomeAssistantSwitch() = default;

    const std::string &GetEntityId() const { return entityId_; }

    SwitchState GetState() override
    {
        // Implementation to get state from Home Assistant
        return SwitchState::OFF; // Placeholder
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
        std::cout << "Calling Home Assistant service: " << creds_.GetUrl() << std::endl;
        std::cout << "Bearer token: " << creds_.GetToken() << std::endl;

        static CurlWrapper curl_wrapper;
        static bool curl_initialized = false;

        if (!curl_initialized)
        {
            curl_initialized = curl_wrapper.initialize();
            if (!curl_initialized)
            {
                std::cerr << "Failed to initialize libcurl - functionality disabled" << std::endl;
                return;
            }
        }

        // Prepare JSON data
        std::string jsonData = "{\"entity_id\": \"" + entityId_ + "\"}";

        // Prepare headers
        struct curl_slist *headers = nullptr;
        std::string authHeader = "Authorization: Bearer " + creds_.GetToken();
        headers = curl_wrapper.slist_append(headers, authHeader.c_str());
        headers = curl_wrapper.slist_append(headers, "Content-Type: application/json");

        CURL *curl = curl_wrapper.easy_init();
        if (curl)
        {
            std::string url = creds_.GetUrl() + "/api/services/" + action;
            std::cout << "HomeAssistantSwitch: URL: " << url << std::endl;
            curl_wrapper.easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_wrapper.easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
            curl_wrapper.easy_setopt(curl, CURLOPT_POSTFIELDS, jsonData.c_str());

            CURLcode res = curl_wrapper.easy_perform(curl);
            if (res != CURLE_OK)
            {
                std::cerr << "curl_easy_perform() failed: " << curl_wrapper.easy_strerror(res) << std::endl;
            }

            curl_wrapper.easy_cleanup(curl);
            curl_wrapper.slist_free_all(headers);
            std::cout << "\nHomeAssistantSwitch: Service call completed" << std::endl;
        }
    }
};