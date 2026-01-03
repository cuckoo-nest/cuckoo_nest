#include "Inputs.hpp"
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <stdio.h>

Inputs::Inputs(std::string button_path, std::string rotary_path) : 
    button_path_(button_path), rotary_path_(rotary_path), button_fd_(-1), rotary_fd_(-1), should_stop_(false)
{
}

Inputs::~Inputs()
{
    stop_polling();
    
    if (button_fd_ != -1) {
        close(button_fd_);
    }
    if (rotary_fd_ != -1) {
        close(rotary_fd_);
    }
}

bool Inputs::initialize()
{
    button_fd_ = open(button_path_.c_str(), O_RDONLY | O_NONBLOCK);
    if (button_fd_ == -1) {
        perror("Failed to open button input");
        return false;
    }

    rotary_fd_ = open(rotary_path_.c_str(), O_RDONLY | O_NONBLOCK);
    if (rotary_fd_ == -1) {
        perror("Failed to open rotary input");
        close(button_fd_);
        return false;
    }
    
    return true;
}

bool Inputs::start_polling()
{
    if (!initialize()) {
        return false;
    }
    
    should_stop_ = false;
    polling_thread_ = std::thread(&Inputs::polling_loop, this);
    return true;
}

void Inputs::stop_polling()
{
    should_stop_ = true;
    if (polling_thread_.joinable()) {
        polling_thread_.join();
    }
}

void Inputs::polling_loop()
{
    while (!should_stop_) 
    {
        bool button_event = poll_device(InputDeviceType::BUTTON, button_fd_);
        bool rotary_event = poll_device(InputDeviceType::ROTARY, rotary_fd_);

        if (rotary_event) {
            // If a rotary event was processed, read again quickly to catch all events
            usleep(1 * 1000); // 1ms
            continue;
        }

        // Small sleep to prevent excessive CPU usage
        usleep(10 * 1000); // 10ms
    }
}

bool Inputs::poll_device(const InputDeviceType device_type, int fd)
{
    struct input_event event;
    memset(&event, 0, sizeof(event));
    
    int bytes_read = read(fd, &event, sizeof(event));
    if (bytes_read > 0 && callback_) {
        callback_(device_type, event);
        return true;
    }

    return false;
}