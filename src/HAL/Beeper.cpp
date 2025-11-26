#include "Beeper.hpp"
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include <cstring>
#include <spdlog/spdlog.h>

Beeper::Beeper(std::string device_path) : device_path_(device_path)
{
}

Beeper::~Beeper()
{
}

void Beeper::play(int duration_ms)
{
    // Open the beeper device
    int fd = open(device_path_.c_str(), O_WRONLY);
    if (fd < 0)
    {
        spdlog::error("Failed to open beeper device");
        return;
    }

    struct input_event beep_start;
    memset(&beep_start, 0, sizeof(beep_start));
    beep_start.type = EV_SND;
    beep_start.code = SND_BELL;
    beep_start.value = 1000;
    write(fd, &beep_start, sizeof(beep_start));

    
    usleep(duration_ms * 1000); // Beep duration 200 ms
    
    struct input_event beep_stop;
    memset(&beep_stop, 0, sizeof(beep_stop));
    beep_stop.type = EV_SND;
    beep_stop.code = SND_BELL;
    beep_stop.value = 0;
    write(fd, &beep_stop, sizeof(beep_stop));
    
    close(fd);
}