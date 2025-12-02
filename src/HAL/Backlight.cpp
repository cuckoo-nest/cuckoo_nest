#include "Backlight.hpp"
#include "logger.h"
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
        LOG_ERROR_STREAM("Failed to open brightness device. Err: " << strerror(errno));
        return;
    }

    fprintf(f, "%d\n", brightness);

    LOG_INFO_STREAM("Brightness set to: " << brightness);

    fclose(f);
}


void Backlight::Activate()
{
    std::lock_guard<std::mutex> lock(mutex_);
    set_backlight_brightness(max_brightness_);
    remaining_seconds_ = active_seconds_;
}

void Backlight::Tick()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (remaining_seconds_ > 0)
    {
        --remaining_seconds_;
        if (remaining_seconds_ == 0)
        {
            set_backlight_brightness(min_brightness_);
        }
    }
}

void Backlight::set_active_seconds(int secs)
{
    std::lock_guard<std::mutex> lock(mutex_);
    active_seconds_ = secs;
}

void Backlight::set_max_brightness(int brightness)
{
    std::lock_guard<std::mutex> lock(mutex_);
    max_brightness_ = brightness;
}

void Backlight::set_min_brightness(int brightness)
{
    std::lock_guard<std::mutex> lock(mutex_);
    min_brightness_ = brightness;
}

