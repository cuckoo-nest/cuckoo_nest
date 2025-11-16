#pragma once
#include <string>

class Backlight
{
public:
    Backlight(std::string device_path);
    ~Backlight();
    void set_backlight_brightness(int brightness);

private:
    std::string device_path_;
};