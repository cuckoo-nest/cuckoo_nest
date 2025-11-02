#pragma once
#include <string>
#include <stdint.h>

class Display {
public:
    Display(std::string device_path);
    bool initialize();
    void change_color(uint32_t color);

private:
    std::string device_path_;
    int screen_buffer;
};