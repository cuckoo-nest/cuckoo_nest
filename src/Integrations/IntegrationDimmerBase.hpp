#pragma once
#include <string>

class IntegrationDimmerBase 
{
    public:
        virtual ~IntegrationDimmerBase() = default;

        virtual SwitchState GetState() = 0;
        virtual void TurnOn() = 0;
        virtual void TurnOff() = 0;

        virtual void SetBrightness(int brightness) = 0;
        virtual int GetBrightness() = 0;

        int GetId() const { return id; }
        void SetId(int newId) { id = newId; }

        std::string GetName() const { return name; }
        void SetName(const std::string& newName) { name = newName; }

    private:
        int id;
        std::string name;
};
