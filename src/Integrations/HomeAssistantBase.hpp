#pragma once
#include <string>

#include "IntegrationSwitchBase.hpp"
#include "HomeAssistantCreds.hpp"
#include "CurlWrapperJson.hpp"

#include <json11.hpp>

class HomeAssistantBase
{
public:
    HomeAssistantBase() = default;
    virtual ~HomeAssistantBase() = default;

    json11::Json queryStatus(std::string id, HomeAssistantCreds &creds)
    {
        std::string url = creds.GetUrl() + "/api/states/" + id;
        return cwj_.Bearer(creds.GetToken())->jsonGetOrPost(url);
    }

    json11::Json queryExecute(std::string const &action, std::string jsonData, HomeAssistantCreds &creds)
    {
        std::string url = creds.GetUrl() + "/api/services/" + action;
        return cwj_.Bearer(creds.GetToken())->jsonGetOrPost(url, jsonData);
    }

protected:
    CurlWrapperJson cwj_;
};

class HomeAssistantDimmerBase : public HomeAssistantBase, IntegrationDimmerBase
{
public:
    HomeAssistantDimmerBase() = default;
    virtual ~HomeAssistantDimmerBase() = default;
};
