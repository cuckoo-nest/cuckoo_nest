#pragma once

#include "BackplateComms.hpp"
#include <string>
#include <vector>

class UnixSerialPort : public ISerialPort {
public:
    explicit UnixSerialPort(const std::string &port);
    virtual ~UnixSerialPort();

    bool Open(BaudRate baudRate) override;
    void Close() override;
    int Read(char* buffer, int bufferSize) override;
    int Write(const std::vector<uint8_t> &data) override;
    int SendBreak(int durationMs) override;
    int Flush() override;

private:
    std::string portName;
    int fd = -1;
};
