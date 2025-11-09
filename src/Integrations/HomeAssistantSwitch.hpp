#pragma once

#include "IntegrationSwitchBase.hpp"
#include "HomeAssistantCreds.hpp"

class HomeAssistantSwitch : public IntegrationSwitchBase 
{
    public:
        HomeAssistantSwitch(const HomeAssistantCreds& creds, const std::string& entity_id) : 
            creds_(creds), 
            entity_id_(entity_id) 
            {}

        virtual ~HomeAssistantSwitch() = default;

        const std::string& GetEntityId() const { return entity_id_; }

        SwitchState GetState() override 
        {
            // Implementation to get state from Home Assistant
            return SwitchState::OFF; // Placeholder
        }

        void TurnOn() override 
        {
            // Implementation to turn on the switch via Home Assistant
        }

        void TurnOff() override 
        {
            // Implementation to turn off the switch via Home Assistant
        }

    private:
        HomeAssistantCreds creds_;
        std::string entity_id_;
        
};