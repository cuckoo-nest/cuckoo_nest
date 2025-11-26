#include "IntegrationContainer.hpp"
#include <fstream>
#include <sstream>
#include <json11.hpp>
#include <spdlog/spdlog.h>
#include <memory>
#include <stdio.h>

#include "HomeAssistantSwitch.hpp"
#include "HomeAssistantDimmer.hpp"

IntegrationContainer::IntegrationContainer()
{
}

void IntegrationContainer::LoadIntegrationsFromConfig(const std::string& configPath)
{
    auto configContent = ReadFileContents(configPath);
    if (configContent.empty())
    {
        // Handle error: could not read config file
        spdlog::error("ConfigurationReader: Could not read config file");
        return;
    }

    std::string parse_error;
    json11::Json parsed_json = json11::Json::parse(configContent, parse_error);

    if (!parse_error.empty()) {
        spdlog::error("ConfigurationReader: JSON parse error: {}", parse_error);
        return;
    }
    
    if (!parsed_json.is_object()) {
        spdlog::error("ConfigurationReader: Root JSON element must be an object");
        return;
    }

    // from config we want to:
    // exract global home assistant settings (token, baseurl)
    homeAssistantCreds_ = HomeAssistantCreds(
        parsed_json["homeAssistant"]["baseURL"].string_value(),
        parsed_json["homeAssistant"]["token"].string_value()
    );

    for (const auto& integration : parsed_json["integrations"].array_items()) 
    {
        if (!integration.is_object()) {
            continue; // skip invalid entries
        }

        int id = integration["id"].int_value();
        std::string name = integration["name"].string_value();
        std::string type = integration["type"].string_value();
        
        if (type == "HomeAssistant") 
        {
            std::string entityId = integration["entityId"].string_value();
            std::string domain = entityId.substr(0, entityId.find('.'));
            if (domain == "switch") 
            {
                auto switchPtr = std::unique_ptr<HomeAssistantSwitch>(
                    new HomeAssistantSwitch(homeAssistantCreds_, entityId)
                );
                switchPtr->SetId(id);
                switchPtr->SetName(name);
                switchMap_[id] = std::move(switchPtr);
            } 
            else if (domain == "light") 
            {
                auto dimmerPtr = std::unique_ptr<HomeAssistantDimmer>(
                    new HomeAssistantDimmer(homeAssistantCreds_, entityId)
                );
                dimmerPtr->SetId(id);
                dimmerPtr->SetName(name);
                dimmerMap_[id] = std::move(dimmerPtr);
            }
        }
    }

}

std::string IntegrationContainer::ReadFileContents(const std::string& filepath) const
{
    std::ifstream file(filepath);
    if (!file.is_open()) {
        return "";
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();

    return buffer.str();
}

IntegrationSwitchBase* IntegrationContainer::GetSwitchById(int id)
{
    auto it = switchMap_.find(id);
    if (it != switchMap_.end())
    {
        return it->second.get();
    }
    return nullptr;
}

IntegrationDimmerBase* IntegrationContainer::GetDimmerById(int id)
{
    auto it = dimmerMap_.find(id);
    if (it != dimmerMap_.end())
    {
        return it->second.get();
    }
    return nullptr;
}