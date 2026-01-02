#pragma once
#include <string>

class IntegrationDimmerBase : public IntegrationSwitchBase
{
    public:
        IntegrationDimmerBase() = default;
        virtual ~IntegrationDimmerBase() = default;

        virtual void SetBrightness(int brightness) = 0;
        virtual int GetBrightness() = 0;
};
