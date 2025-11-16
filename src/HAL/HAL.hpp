#pragma once

#include "IDisplay.hpp"
#include "Beeper.hpp"
#include "Inputs.hpp"
#include "Backlight.hpp"

class HAL 
{
public:
    HAL() = default;
    virtual ~HAL() = default;

    IDisplay* display = nullptr;
    Beeper* beeper = nullptr;
    Inputs* inputs = nullptr;
    Backlight* backlight = nullptr;
};