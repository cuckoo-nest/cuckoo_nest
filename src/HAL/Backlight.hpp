#pragma once
#include <string>
#include <mutex>

class Backlight
{
public:
    Backlight(std::string device_path);
    ~Backlight();

    // Immediate set of brightness (writes to sysfs)
    void set_backlight_brightness(int brightness);

    // Activate the backlight for the configured active duration
    void Activate();

    // Should be called once-per-second from main loop to manage timeout
    void Tick();

    // Setters for behavior
    void set_active_seconds(int secs);
    void set_max_brightness(int brightness);
    void set_min_brightness(int brightness);

private:
    std::string device_path_;
    std::mutex mutex_;

    int active_seconds_{10};
    int remaining_seconds_{0};
    int max_brightness_{115};
    int min_brightness_{20};
};