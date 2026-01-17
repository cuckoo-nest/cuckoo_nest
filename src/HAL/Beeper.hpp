#pragma once
#include <string>
class Beeper 
{
public:
    Beeper(std::string device_path) : device_path_(device_path) {};
    virtual ~Beeper() {};
    void play(int duration_ms);
    void click();

private:
    void execute(struct input_event const &beep_start, int duration_ms, struct input_event const &beep_stop);

    std::string device_path_;
};