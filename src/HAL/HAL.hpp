#pragma once

#include "Display.hpp"
#include "Beeper.hpp"
#include "Inputs.hpp"

class HAL 
{
public:
    HAL() = default;
    virtual ~HAL() = default;

    Display* display = nullptr;
    Beeper* beeper = nullptr;
    Inputs* inputs = nullptr;
};