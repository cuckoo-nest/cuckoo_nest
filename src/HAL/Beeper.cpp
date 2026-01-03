#include "Beeper.hpp"
#include <fcntl.h>
#include <unistd.h>
#include "InputEvent.hpp"
#include <cstring>
#include "logger.h"

void Beeper::execute(struct input_event const &beep_start, int duration_ms, struct input_event const &beep_stop)
{
    // Open the beeper device
    int fd = open(device_path_.c_str(), O_WRONLY);
    if (fd < 0)
    {
        LOG_ERROR_STREAM("Failed to open beeper device");
        return;
    }

    write(fd, &beep_start, sizeof(beep_start));
    usleep(duration_ms * 1000); // Beep duration 200 ms
    write(fd, &beep_stop, sizeof(beep_stop));

    close(fd);
}

void Beeper::play(int duration_ms)
{
    struct input_event beep_start = {0};
    struct input_event beep_stop = {0};

    beep_start.type = EV_SND;
    beep_start.code = SND_BELL;
    beep_start.value = 1000;

    beep_stop.type = EV_SND;
    beep_stop.code = SND_BELL;
    beep_stop.value = 0;

    execute(beep_start, duration_ms, beep_stop);
}

void Beeper::click()
{
    struct input_event beep_start = {0};
    struct input_event beep_stop = {0};

    beep_start.type = EV_SND;
    beep_start.code = SND_BELL;
    beep_start.value = 3;

    beep_stop.type = EV_SND;
    beep_stop.code = SND_BELL;
    beep_stop.value = 0;
    execute(beep_start, 5, beep_stop);
}