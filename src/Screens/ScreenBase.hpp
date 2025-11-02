#pragma once
#include "../HAL/InputDevices.hxx"
#include <linux/input.h>

class ScreenBase
{
public:
    virtual ~ScreenBase() = default;

    virtual void Render() = 0;
    virtual void handle_input_event(const InputDeviceType device_type, const struct input_event& event) = 0;
};