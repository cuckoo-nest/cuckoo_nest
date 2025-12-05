#pragma once

#include <linux/input.h>
#include "HAL/InputDevices.hxx"

class InputEvent{
public:
    InputEvent(InputDeviceType device_type, const struct input_event &event)
        : device_type(device_type), event(event) {}
    InputDeviceType device_type;
    struct input_event event;
};