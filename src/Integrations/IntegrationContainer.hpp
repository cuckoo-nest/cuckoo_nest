#pragma once
#include <string>
#include <memory>
#include <map>

#include "IntegrationSwitchBase.hpp"
#include "IntegrationDimmerBase.hpp"
#include "HomeAssistantCreds.hpp"


class IntegrationContainer 
{
    public:
        IntegrationContainer() {};
        virtual ~IntegrationContainer() = default;
        void LoadIntegrationsFromConfig(const std::string& configPath);

        
        IntegrationSwitchBase* GetSwitchById(std::string const  &id);
        IntegrationDimmerBase* GetDimmerById(std::string const  &id);
        
    private:
        std::string ReadFileContents(const std::string &filepath) const;
        
        std::map<std::string, std::unique_ptr<IntegrationSwitchBase>> switchMap_;
        std::map<std::string, std::unique_ptr<IntegrationDimmerBase>> dimmerMap_;

    private:
        HomeAssistantCreds homeAssistantCreds_;

};