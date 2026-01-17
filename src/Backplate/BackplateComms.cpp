#include <mutex>
#include <unistd.h>
#include <string>
#include <cstring>
#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>
#include <ctype.h>

#include "logger.h"
#include "BackplateComms.hpp"
#include "CommandMessage.hpp"
#include "ResponseMessage.hpp"
#include "MessageParser.hpp"
#include "CTick.hpp"

BackplateComms::BackplateComms(ISerialPort* serialPort, IDateTimeProvider* dateTimeProvider)
    : SerialPort(serialPort), DateTimeProvider(dateTimeProvider)
{
    this->running.store(false);
    this->LastKeepAliveTime.tv_sec = 0;
    this->LastKeepAliveTime.tv_usec = 0;
    this->LastHistoricalDataRequestTime.tv_sec = 0;
    this->LastHistoricalDataRequestTime.tv_usec = 0;
}

BackplateComms::~BackplateComms()
{
    // Stop background thread and join
    running.store(false);
    if (workerThread.joinable())
    {
        workerThread.join();
    }
}

bool BackplateComms::Initialize()
{
    LOG_DEBUG_STREAM("BackplateComms: Initializing...");

    bool success = false;
    if (!running.load())
    {
        running.store(true);
        workerThread = std::thread([this](){ this->TaskBodyRunningState(); });
        success = true;
    }

    return success;
}

void BackplateComms::TaskBodyRunningState()
{
    CTickFuture tickRunState(10);
    while(running.load())
    {
        if(runstate_ < 99)
        {
            if(tickRunState.IsExpired())
            {
                tickRunState.Reset();
                switch(runstate_)
                {
                    case 0:
                        if(!InitializeSerial())
                        {
                            tickRunState.ScheduleSec(30);
                            LOG_ERROR_STREAM("BackplateComms: Failed to initialize serial port.");
                        }
                        else
                        {
                            LOG_INFO_STREAM("BackplateComms: serial port initialized.");
                            runstate_ ++;
                        }
                        break;
                    case 1:
                        if(!DoBurstStage())
                        {
                            tickRunState.ScheduleSec(30);
                            LOG_ERROR_STREAM("BackplateComms: Burst stage failed.");
                            runstate_ = 0;
                        }
                        else
                        {
                            LOG_INFO_STREAM("BackplateComms: Burst stage success.");
                            runstate_ ++;
                        }
                        break;
                    case 2:
                        if(!DoInfoGathering())
                        {
                            tickRunState.ScheduleSec(30);
                            LOG_ERROR_STREAM("BackplateComms: Info gathering failed.");
                            runstate_ = 0;
                        }
                        else
                        {
                            LOG_INFO_STREAM("BackplateComms: Info gathering success.");
                            runstate_ ++;
                        }
                        break;
                    default:
                        LOG_INFO_STREAM("BackplateComms: Transition to normal comms.");
                        runstate_ = 99;
                }
            }
        }
        else
        {
            TaskBodyComms();
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

bool BackplateComms::InitializeSerial()
{
    const BaudRate baudRate = BaudRate::Baud115200;

    SerialPort->Close();
    if (!SerialPort->Open(baudRate))
    {
        return false;
    }

    SerialPort->Flush();
    SerialPort->SendBreak(0);
    usleep(250000);
    SerialPort->Flush();

    return true;
}

bool BackplateComms::DoBurstStage()
{
    CommandMessage resetMsg(MessageType::Reset);
    SerialPort->Write(resetMsg.GetRawMessage());

    // Wait for 5s for backplate to send a burst of data
    // there will be a bunch of data points, ending with a BRK
    bool burstDone = false;
    bool fet_data_received = false;
    std::vector<uint8_t> fetPresencePayload;
    struct timeval startTime;
    DateTimeProvider->gettimeofday(startTime);

    uint8_t tmpBuffer[256];
    size_t totalBytesRead = 0;

    while (true)
    {
        if (IsTimeout(startTime, BurstTimeoutUs))
        {
            LOG_ERROR_STREAM("Burst stage timed out.");
            break;
        }

        int bytesRead = SerialPort->Read(
            reinterpret_cast<char *>(tmpBuffer),
            sizeof(tmpBuffer));

        LOG_DEBUG_STREAM("Read " << bytesRead << " bytes");

        if (bytesRead == 0)
        {
            LOG_DEBUG_STREAM("No data read, continuing...");
            usleep(100000); // 100ms
            continue;
        }

        totalBytesRead += bytesRead;

        // Feed into parser
        auto msgs = parser.Feed(tmpBuffer, bytesRead);
        for (auto &responseMsg : msgs)
        {
            if (responseMsg.GetMessageCommand() == MessageType::FetPresenceData)
            {
                fetPresencePayload = responseMsg.GetPayload();
                fet_data_received = true;
                LOG_DEBUG_STREAM("FET presence data received.");
            }

            const std::vector<uint8_t> brk = {'B', 'R', 'K'};
            if (responseMsg.GetMessageCommand() == MessageType::ResponseAscii 
            && responseMsg.GetPayload() == brk)
            {
                burstDone = true;
                break;
            }
        }
        if (burstDone)
        {
            break;
        }
    }

    if (!burstDone)
    {
        LOG_ERROR_STREAM("Handshake Error: Did not receive 'BRK' signal.");
        return false;
    }

    if (!fet_data_received)
    {
        LOG_ERROR_STREAM("Handshake Error: Did not receive FET presence data.");
        return false;
    }

    // now send Ack for the FET presence data
    CommandMessage ackMsg(MessageType::FetPresenceAck);
    ackMsg.SetPayload(fetPresencePayload);
    SerialPort->Write(ackMsg.GetRawMessage());
    return true;
}

bool BackplateComms::IsTimeout(timeval &startTime, int timeoutUs)
{
    timeval currentTime;
    DateTimeProvider->gettimeofday(currentTime);
    long elapsedUs = (currentTime.tv_sec - startTime.tv_sec) * 1000000 +
                     (currentTime.tv_usec - startTime.tv_usec);

    
    return (elapsedUs >= timeoutUs);
}

bool BackplateComms::DoInfoGathering()
{
    if (!GetInfo(MessageType::GetTfeVersion, MessageType::TfeVersion))
    {
        LOG_ERROR_STREAM("Failed to get TfeVersion");
        return false;
    }

    if (!GetInfo(MessageType::GetTfeBuildInfo, MessageType::TfeBuildInfo))
    {
        LOG_ERROR_STREAM("Failed to get TfeBuildInfo");
        return false;
    }

    if (!GetInfo(MessageType::GetBackplateModelAndBslId, MessageType::BackplateModelAndBslId))
    {
        LOG_ERROR_STREAM("Failed to get BackplateModelAndBslId");
        return false;
    }

    return true;
}

bool BackplateComms::GetInfo(MessageType command, MessageType expectedResponse)
{
    CommandMessage commandMsg(command);
    SerialPort->Write(commandMsg.GetRawMessage());

    struct timeval startTime;
    DateTimeProvider->gettimeofday(startTime);

    while (true)
    {
        if (IsTimeout(startTime, GetInfoTimeoutUs))
        {
            break;
        }

        uint8_t readBuffer[256];
        int bytesRead = SerialPort->Read(
            reinterpret_cast<char *>(readBuffer),
            sizeof(readBuffer));

        if (bytesRead == 0)
        {
            usleep(100000); // 100ms
            continue;
        }

        auto msgs = parser.Feed(readBuffer, bytesRead);
        for (auto &responseMsg : msgs)
        {
            LOG_DEBUG_STREAM("GetInfo: Received response for command " << static_cast<uint16_t>(responseMsg.GetMessageCommand()));
            LOG_DEBUG_STREAM("GetInfo: Payload size: " << static_cast<uint32_t>(responseMsg.GetPayload().size()) << " bytes");

            if (responseMsg.GetMessageCommand() == expectedResponse)
            {
                std::cout << "GetInfo: Received expected response for command "
                          << static_cast<uint16_t>(command) << std::endl;
                return true;
            }
        }
    }

    LOG_ERROR_STREAM("GetInfo Error: Did not receive expected response for command 0x" << std::hex << static_cast<uint16_t>(command));
    return false;
}

void BackplateComms::TaskBodyComms ()
{

    if (IsTimeForKeepalive())
    {
        CommandMessage keepAliveMsg(MessageType::PeriodicStatusRequest);
        SerialPort->Write(keepAliveMsg.GetRawMessage());
    }

    if (IsTimeForHistoricalDataRequest())
    {
        CommandMessage historicalDataMsg(MessageType::GetHistoricalDataBuffers);
        SerialPort->Write(historicalDataMsg.GetRawMessage());
    }

    // Read available data and attempt to parse ResponseMessage packets
    uint8_t readBuffer[256];
    int bytesRead = SerialPort->Read(reinterpret_cast<char *>(readBuffer), sizeof(readBuffer));

    if (bytesRead <= 0)
    {
        // nothing to do
        return;
    }

    auto msgs = parser.Feed(readBuffer, bytesRead);
    for (auto &resp : msgs)
    {
        // Successful parse: emit a simple log point for measurements/events
        MessageType cmd = resp.GetMessageCommand();
        switch (cmd)
        {
            case MessageType::TempHumidityData:
                if (resp.GetPayload().size() >= 4)
                {
                    int16_t temp_cc = (resp.GetPayload()[1] << 8) | resp.GetPayload()[0];
                    uint16_t hum_pm = (resp.GetPayload()[3] << 8) | resp.GetPayload()[2];
                    LOG_DEBUG_STREAM("temp_cc=" << temp_cc << " hum_pm=" << hum_pm);
                    {
                        std::lock_guard<std::mutex> lk(this->dataMutex);
                        CurrentTemperatureC = static_cast<double>(temp_cc) / 100.0;
                        CurrentHumidityPercent = static_cast<double>(hum_pm) / 10.0;
                        LOG_INFO("BackplateComms: TempHumidityData: Temperature = %.2f C, Humidity = %.2f %%", CurrentTemperatureC, CurrentHumidityPercent);
                    }

                    for (auto &cb : this->tempCallbacks)
                    {
                        if (cb)
                        {
                            cb(CurrentTemperatureC);
                        }
                    }
                }
                break;

            case MessageType::PirDataRaw:
                LOG_INFO_STREAM("BackplateComms: Received PIR data (raw) size=" << resp.GetPayload().size());
                break;

            case MessageType::AmbientLightSensor:
            {
                uint16_t lux = 0;
                if (resp.GetPayload().size() >= 2)
                {
                    lux = (resp.GetPayload()[1] << 8) | resp.GetPayload()[0];
                }

                //LOG_INFO_STREAM("BackplateComms: Ambient Light Sensor Value = " << lux);
                break;
            }

            case MessageType::PirMotionEvent:
                if (resp.GetPayload().size() >= 4) {
                    int16_t val1 = (resp.GetPayload()[1] << 8) | resp.GetPayload()[0];
                    int16_t val2 = (resp.GetPayload()[3] << 8) | resp.GetPayload()[2];
                    if (val1 == 0 && val2 == 0)
                    {
                        LOG_INFO("PIR Event: Cleared");
                    }
                    else 
                    {
                        LOG_INFO("PIR Event: Motion Detected (vals: %d, %d)", val1, val2);
                    }
                }
                break;

            case MessageType::ProximityEvent:
                LOG_INFO_STREAM("BackplateComms: Received proximity event size=" << resp.GetPayload().size());
                break;

            case MessageType::ProximitySensorHighDetail:
                LOG_INFO_STREAM("BackplateComms: Received proximity sensor (high detail) data size=" << resp.GetPayload().size());
                break;

            case MessageType::BackplateState:
                LOG_INFO_STREAM("BackplateComms: Received backplate state size=" << resp.GetPayload().size());
                break;

            case MessageType::ProxSensor:
                if (resp.GetPayload().size() >= 4) {
                    int16_t val1 = (resp.GetPayload()[1] << 8) | resp.GetPayload()[0];
                    int16_t val2 = (resp.GetPayload()[3] << 8) | resp.GetPayload()[2];
                    LOG_INFO("Proximity Sensor -> Val1: %d, Val2: %d", val1, val2);
                } else if (resp.GetPayload().size() >= 2) { // Handle the 2-byte case we are seeing
                    int16_t val1 = (resp.GetPayload()[1] << 8) | resp.GetPayload()[0];
                    LOG_DEBUG("Proximity Sensor -> Value: %d", val1);
                    for (auto &cb : this->pirCallbacks)
                    {
                        if (cb)
                        {
                            cb(val1);
                        }
                    }
                    
                }
                break;

            case MessageType::RawAdcData:
                if (resp.GetPayload().size() >= 14) {
                    uint16_t pir_raw = (resp.GetPayload()[1] << 8) | resp.GetPayload()[0];
                    uint16_t alir_raw = (resp.GetPayload()[11] << 8) | resp.GetPayload()[10];
                    uint16_t alvis_raw = (resp.GetPayload()[13] << 8) | resp.GetPayload()[12];
                    LOG_INFO("Sensor ADC -> PIR: %u, AL_IR: %u, AL_VIS: %u", pir_raw, alir_raw, alvis_raw);
                }
                break;

            default:
                LOG_INFO_STREAM("BackplateComms: Received cmd=0x" << std::hex << static_cast<uint16_t>(cmd) << " size=" << resp.GetPayload().size());
                break;
        }

        // Invoke any subscribed callbacks (multi-subscriber std::function lists)
        const std::vector<uint8_t> &payload = resp.GetPayload();
        const uint8_t* payloadPtr = payload.empty() ? nullptr : payload.data();

        for (auto &cb : this->genericCallbacks)
        {
            if (cb)
            {
                cb(static_cast<uint16_t>(cmd), payloadPtr, payload.size());
            }
        }
    }

}

bool BackplateComms::IsTimeForKeepalive()
{
    struct timeval currentTime;
    DateTimeProvider->gettimeofday(currentTime);

    long elapsedSecs = (currentTime.tv_sec - LastKeepAliveTime.tv_sec);

    if (elapsedSecs >= KeepAliveIntervalSeconds
    || LastKeepAliveTime.tv_sec == 0)
    {
        LastKeepAliveTime = currentTime;
        return true;
    }
    return false;
}

bool BackplateComms::IsTimeForHistoricalDataRequest()
{
    struct timeval currentTime;
    DateTimeProvider->gettimeofday(currentTime);

    long elapsedSecs = (currentTime.tv_sec - LastHistoricalDataRequestTime.tv_sec);
    if (elapsedSecs >= HistoricalDataIntervalSeconds
    || LastHistoricalDataRequestTime.tv_sec == 0)
    {
        LastHistoricalDataRequestTime = currentTime;
        return true;
    }
    return false;
}