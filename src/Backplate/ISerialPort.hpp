#pragma once

#include "BaudRate.hpp"
#include <string>
#include <vector>

class ISerialPort {
public:
    explicit ISerialPort(std::string portName)
        : portName(std::move(portName)) {}

    virtual ~ISerialPort() = default;
    
    virtual bool Open(BaudRate baudRate) = 0;
    virtual void Close() = 0;
    virtual int Read(char* buffer, int bufferSize) = 0;
    virtual int Write(const std::vector<uint8_t> &data) = 0;
    virtual int SendBreak(int durationMs) = 0;
    virtual int Flush() = 0;

private:
    std::string portName;
};