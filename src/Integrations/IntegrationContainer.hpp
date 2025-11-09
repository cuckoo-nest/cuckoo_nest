#pragma once
#include <string>
#include <memory>
#include <map>

#include "IntegrationSwitchBase.hpp"
#include "HomeAssistantCreds.hpp"


class IntegrationContainer 
{
    public:
        IntegrationContainer();
        virtual ~IntegrationContainer() = default;
        void LoadIntegrationsFromConfig(const std::string& configPath);

        
        IntegrationSwitchBase* GetSwitchById(int id);
        
        private:
        std::string ReadFileContents(const std::string &filepath) const;
        
        std::map<int, std::unique_ptr<IntegrationSwitchBase>> switchMap_;

    private:
        HomeAssistantCreds homeAssistantCreds_;

};