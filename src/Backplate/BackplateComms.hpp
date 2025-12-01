#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <functional>
#include <utility>
#include <sys/time.h>
#include <thread>
#include <atomic>
#include "MessageType.hxx"
#include "../IDateTimeProvider.hpp"
#include "MessageParser.hpp"

enum class BaudRate {
    Baud9600 = 9600,
    Baud19200 = 19200,
    Baud38400 = 38400,
    Baud57600 = 57600,
    Baud115200 = 115200
};

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

class BackplateComms {
public:
    BackplateComms(
        ISerialPort* serialPort,
        IDateTimeProvider* dateTimeProvider);

    virtual ~BackplateComms();

    bool Initialize();
    bool InitializeSerial();
    bool DoBurstStage();
    bool DoInfoGathering();
    bool GetInfo(MessageType command, MessageType expectedResponse);
    void MainTaskBody(void);

    bool IsTimeForKeepalive();
    bool IsTimeForHistoricalDataRequest();

    // Multi-subscriber event subscriptions using std::function
    using TemperatureCallback = std::function<void(const uint8_t* payload, size_t length)>;
    using PIRCallback = std::function<void(const uint8_t* payload, size_t length)>;
    using GenericEventCallback = std::function<void(uint16_t messageType, const uint8_t* payload, size_t length)>;

    // Add a subscriber; returns an index token (size_t) that can be used with Clear*Callbacks
    size_t AddTemperatureCallback(TemperatureCallback cb) { tempCallbacks.push_back(std::move(cb)); return tempCallbacks.size()-1; }
    size_t AddPIRCallback(PIRCallback cb) { pirCallbacks.push_back(std::move(cb)); return pirCallbacks.size()-1; }
    size_t AddGenericEventCallback(GenericEventCallback cb) { genericCallbacks.push_back(std::move(cb)); return genericCallbacks.size()-1; }

    // Clear all subscribers for a type
    void ClearTemperatureCallbacks() { tempCallbacks.clear(); }
    void ClearPIRCallbacks() { pirCallbacks.clear(); }
    void ClearGenericEventCallbacks() { genericCallbacks.clear(); }

private:
    bool IsTimeout(timeval &startTime, int timeoutUs);

    // Background worker thread that runs MainTaskBody periodically
    std::thread workerThread;
    std::atomic<bool> running;

private:
    const int KeepAliveIntervalSeconds = 15;
    const int HistoricalDataIntervalSeconds = 60;
    const int BurstTimeoutUs = 5000000;
    const int GetInfoTimeoutUs = 200000;

    ISerialPort* SerialPort;
    IDateTimeProvider* DateTimeProvider;
    timeval LastKeepAliveTime;
    timeval LastHistoricalDataRequestTime;
    // callback lists
    std::vector<TemperatureCallback> tempCallbacks;
    std::vector<PIRCallback> pirCallbacks;
    std::vector<GenericEventCallback> genericCallbacks;
    // Parser for incoming serial bytes
    MessageParser parser;
};

    inline BackplateComms::BackplateComms(ISerialPort* serialPort, IDateTimeProvider* dateTimeProvider)
        : SerialPort(serialPort), DateTimeProvider(dateTimeProvider)
    {
        this->running.store(false);
        this->LastKeepAliveTime.tv_sec = 0;
        this->LastKeepAliveTime.tv_usec = 0;
        this->LastHistoricalDataRequestTime.tv_sec = 0;
        this->LastHistoricalDataRequestTime.tv_usec = 0;
    }
