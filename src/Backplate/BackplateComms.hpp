#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include "MessageType.hxx"
#include "../IDateTimeProvider.hpp"

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
        IDateTimeProvider* dateTimeProvider) : 
        SerialPort(serialPort),
        DateTimeProvider(dateTimeProvider) {}
    
    virtual ~BackplateComms() = default;

    bool Initialize();
    bool InitializeSerial();
    bool DoBurstStage();
    bool DoInfoGathering();
    bool GetInfo(MessageType command, MessageType expectedResponse);
    void MainTaskBody(void);

    bool IsTimeForKeepalive();
    bool IsTimeForHistoricalDataRequest();

private:
    bool IsTimeout(timeval &startTime, int timeoutUs);


private:
    const int KeepAliveIntervalSeconds = 15;
    const int HistoricalDataIntervalSeconds = 60;
    const int BurstTimeoutUs = 5000000;
    const int GetInfoTimeoutUs = 200000;

    ISerialPort* SerialPort;
    IDateTimeProvider* DateTimeProvider;
    timeval LastKeepAliveTime {0, 0};
    timeval LastHistoricalDataRequestTime {0, 0};
};
