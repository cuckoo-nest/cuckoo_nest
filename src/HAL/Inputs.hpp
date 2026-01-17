#pragma once
#include <string>
#include <thread>
#include <atomic>
#include <functional>
#include "InputEvent.hpp"
#include "InputDevices.hxx"

class Inputs {
public:
    // Callback function type for input events
    using InputCallback = 
        std::function<void(
            const InputDeviceType device_type, 
            const struct input_event& event)
        >;
    
    Inputs(std::string button_path, std::string rotary_path);
    ~Inputs();
    
    bool initialize();
    bool start_polling();
    void stop_polling();
    
    // Set callback for input events
    inline void set_callback(InputCallback callback) { callback_ = callback; };

private:
    std::string button_path_;
    std::string rotary_path_;
    int button_fd_;
    int rotary_fd_;
    
    // Threading members
    std::thread polling_thread_;
    std::atomic<bool> should_stop_;
    InputCallback callback_;
    
    // Polling function that runs in background thread
    void polling_loop();
    bool poll_device(const InputDeviceType device_type, int fd);
};