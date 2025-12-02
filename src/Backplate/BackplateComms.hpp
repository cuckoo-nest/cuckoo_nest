#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <functional>
#include <utility>
#include <sys/time.h>
#include <thread>
#include <atomic>
#include <mutex>
#include "MessageType.hxx"
#include "../IDateTimeProvider.hpp"
#include "MessageParser.hpp"
#include "ISerialPort.hpp"

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
    using TemperatureCallback = std::function<void(float temperatureC)>;
    using PIRCallback = std::function<void(int value)>;
    using GenericEventCallback = std::function<void(uint16_t messageType, const uint8_t* payload, size_t length)>;

    // Add a subscriber; returns an index token (size_t) that can be used with Clear*Callbacks
    size_t AddTemperatureCallback(TemperatureCallback cb) { tempCallbacks.push_back(std::move(cb)); return tempCallbacks.size()-1; }
    size_t AddPIRCallback(PIRCallback cb) { pirCallbacks.push_back(std::move(cb)); return pirCallbacks.size()-1; }
    size_t AddGenericEventCallback(GenericEventCallback cb) { genericCallbacks.push_back(std::move(cb)); return genericCallbacks.size()-1; }

    // Clear all subscribers for a type
    void ClearTemperatureCallbacks() { tempCallbacks.clear(); }
    void ClearPIRCallbacks() { pirCallbacks.clear(); }
    void ClearGenericEventCallbacks() { genericCallbacks.clear(); }

    double GetCurrentTemperatureC() const { std::lock_guard<std::mutex> lk(dataMutex); return CurrentTemperatureC; }
    double GetCurrentHumidityPercent() const { std::lock_guard<std::mutex> lk(dataMutex); return CurrentHumidityPercent; }

private:
    bool IsTimeout(timeval &startTime, int timeoutUs);

    // Background worker thread that runs MainTaskBody periodically
    std::thread workerThread;
    std::atomic<bool> running;

    double CurrentTemperatureC = 0.0f;
    double CurrentHumidityPercent = 0.0;
    mutable std::mutex dataMutex;

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
