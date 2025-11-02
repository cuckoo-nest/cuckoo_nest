#pragma once
#include <string>
#include <stdint.h>

class Display {
public:
    Display(std::string device_path);
    ~Display();
    bool initialize();
    void SetBackgroundColor(uint32_t color);

private:
    std::string device_path_;
    int screen_buffer;
    char *fbp;
    long screensize;
};