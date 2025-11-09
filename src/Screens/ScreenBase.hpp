#pragma once
#include "../HAL/InputDevices.hxx"
#include <linux/input.h>

enum screen_color
{
   SCREEN_COLOR_BLACK = 0x000000,
   SCREEN_COLOR_WHITE = 0xFFFFFF,
   SCREEN_COLOR_RED   = 0xFF0000,
   SCREEN_COLOR_GREEN = 0x00FF00,
   SCREEN_COLOR_BLUE  = 0x0000FF
};

class ScreenBase
{
public:
    virtual ~ScreenBase() = default;

    virtual void Render() = 0;
    virtual void handle_input_event(const InputDeviceType device_type, const struct input_event& event) = 0;

    int GetId() const { return id_; }
    void SetId(int id) { id_ = id; }

private:
    int id_;
};