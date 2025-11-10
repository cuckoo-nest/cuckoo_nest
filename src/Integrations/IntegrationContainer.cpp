#include "IntegrationContainer.hpp"
#include <fstream>
#include <sstream>
#include <json11.hpp>
#include <iostream>
#include <memory>
#include <stdio.h>

#include "HomeAssistantSwitch.hpp"

IntegrationContainer::IntegrationContainer()
{
}

void IntegrationContainer::LoadIntegrationsFromConfig(const std::string& configPath)
{
    auto configContent = ReadFileContents(configPath);
    if (configContent.empty())
    {
        // Handle error: could not read config file
        std::cerr << "ConfigurationReader: Could not read config file" << std::endl;
        return;
    }

    std::string parse_error;
    json11::Json parsed_json = json11::Json::parse(configContent, parse_error);

    if (!parse_error.empty()) {
        std::cerr << "ConfigurationReader: JSON parse error: " << parse_error << std::endl;
        return;
    }
    
    if (!parsed_json.is_object()) {
        std::cerr << "ConfigurationReader: Root JSON element must be an object" << std::endl;
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

            auto switchPtr = std::unique_ptr<HomeAssistantSwitch>(
                new HomeAssistantSwitch(homeAssistantCreds_, entityId)
            );
            switchPtr->SetId(id);
            switchPtr->SetName(name);
            switchMap_[id] = std::move(switchPtr);
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