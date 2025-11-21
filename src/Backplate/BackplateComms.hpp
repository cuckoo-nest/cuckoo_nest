#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include "MessageType.hxx"

enum class BaudRate {
    Baud9600 = 9600,
    Baud19200 = 19200,
    Baud38400 = 38400,
    Baud57600 = 57600,
    Baud115200 = 115200
};

class ISerialPort {
public:
    ISerialPort(std::string portName)
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

class BackplateComms {
public:
    BackplateComms(
        ISerialPort* serialPort, 
        int burstTimeoutUs = 5000000,
        int getInfoTimeoutUs = 200000) : 
        SerialPort(serialPort),
        BurstTimeoutUs(burstTimeoutUs),
        GetInfoTimeoutUs(getInfoTimeoutUs) {}
    
    virtual ~BackplateComms() = default;

    bool Initialize();
    bool InitializeSerial();
    bool DoBurstStage();
    bool DoInfoGathering();
    bool GetInfo(MessageType command, MessageType expectedResponse);


private:
    bool IsTimeout(timeval &startTime, int timeoutUs);


private:
    ISerialPort* SerialPort;
    int BurstTimeoutUs;
    int GetInfoTimeoutUs;
};
