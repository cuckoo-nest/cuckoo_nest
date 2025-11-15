#include "Backlight.hpp"
#include <iostream>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

Backlight::Backlight(std::string device_path): device_path_(device_path)
{
}

Backlight::~Backlight()
{
}

/**
 * @brief Sets the backlight brightness by writing to the sysfs file.
 *
 * @param brightness The brightness value to set (e.g., 115).
 */
void Backlight::set_backlight_brightness(int brightness)
{
    FILE* f = fopen(device_path_.c_str(), "w");
    if (f == NULL)
    {
        std::cerr << "Failed to open brightness device.  Err: " << strerror(errno) << "\n";
        return;
    }

    fprintf(f, "%d\n", brightness);

    std::cout << "Brightness set to: " << brightness << "\n";

    fclose(f);
}

