#include "BackplateComms.hpp"
#include "CommandMessage.hpp"
#include "ResponseMessage.hpp"
#include <unistd.h>
#include <string>
#include <cstring>
#include <spdlog/spdlog.h>

bool BackplateComms::Initialize() 
{
    if (!InitializeSerial())
    {
        return false;
    }

    if (!DoBurstStage())
        return false;

    if (!DoInfoGathering())
        return false;

    return true;
}


bool BackplateComms::InitializeSerial()
{
    const BaudRate baudRate = BaudRate::Baud115200;

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

    // Wait for 5s for backplate to send a bust of data
    // there will be a bunch of data points, ending with a BRK
    bool burstDone = false;
    bool fet_data_received = false;
    std::vector<uint8_t> fetPresencePayload;
    struct timeval startTime;
    DateTimeProvider->gettimeofday(startTime);

    while (true)
    {
        if (IsTimeout(startTime, BurstTimeoutUs))
        {
            spdlog::error("Burst stage timed out.");
            break;
        }

        uint8_t readBuffer[256];
        size_t totalBytesRead = 0;
        totalBytesRead = SerialPort->Read(
            reinterpret_cast<char *>(readBuffer),
            sizeof(readBuffer));

        spdlog::debug("Read {} bytes", totalBytesRead);

        if (totalBytesRead == 0)
        {
            spdlog::debug("No data read, continuing...");
            usleep(100000); // 100ms
            continue;
        }

        // Parse the response to find the BRK message
        ResponseMessage responseMsg;
        responseMsg.ParseMessage(readBuffer, totalBytesRead);

        if (responseMsg.GetMessageCommand() == MessageType::FetPresenceData)
        {
            fetPresencePayload = responseMsg.GetPayload();
            fet_data_received = true;
        }

        const std::vector<uint8_t> brk = {'B', 'R', 'K'};
        if (responseMsg.GetMessageCommand() == MessageType::ResponseAscii && responseMsg.GetPayload() == brk)
        {
            burstDone = true;
            break;
        }
    }

    if (!burstDone)
    {
        spdlog::error("Handshake Error: Did not receive 'BRK' signal.");
        return false;
    }

    if (!fet_data_received)
    {
        spdlog::error("Handshake Error: Did not receive FET presence data.");
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

    if (elapsedUs >= timeoutUs)
    {
        return true;
    }
    return false;
}

bool BackplateComms::DoInfoGathering()
{
    if (!GetInfo(MessageType::GetTfeVersion, MessageType::TfeVersion))
    {
        spdlog::error("Failed to get TfeVersion");
        return false;
    }

    if (!GetInfo(MessageType::GetTfeBuildInfo, MessageType::TfeBuildInfo))
    {
        spdlog::error("Failed to get TfeBuildInfo");
        return false;
    }

    if (!GetInfo(MessageType::GetBackplateModelAndBslId, MessageType::BackplateModelAndBslId))
    {
        spdlog::error("Failed to get BackplateModelAndBslId");
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
            break;

        uint8_t readBuffer[256];
        size_t totalBytesRead = 0;
        totalBytesRead = SerialPort->Read(
            reinterpret_cast<char *>(readBuffer),
            sizeof(readBuffer));

        if (totalBytesRead == 0)
        {
            usleep(100000); // 100ms
            continue;
        }

        // Parse the response
        ResponseMessage responseMsg;
        if (!responseMsg.ParseMessage(readBuffer, totalBytesRead))
        {
            spdlog::warn("GetInfo: ParseMessage failed");
        }
        spdlog::debug("GetInfo: Received response for command 0x{:04x}", static_cast<uint16_t>(responseMsg.GetMessageCommand()));
        spdlog::debug("GetInfo: Payload size: {} bytes", static_cast<uint32_t>(responseMsg.GetPayload().size()));

        if (responseMsg.GetMessageCommand() == expectedResponse)
        {
            std::cout << "GetInfo: Received expected response for command "
                      << static_cast<uint16_t>(command) << std::endl;
            return true;
        }
    }

    spdlog::error("GetInfo Error: Did not receive expected response for command 0x{:04x}", static_cast<uint16_t>(command));
    return false;
}

void BackplateComms::MainTaskBody (void)
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
    static std::vector<uint8_t> rxBuffer;
    uint8_t readBuffer[256];
    int bytesRead = SerialPort->Read(reinterpret_cast<char *>(readBuffer), sizeof(readBuffer));

    if (bytesRead <= 0)
    {
        // nothing to do
        return;
    }

    rxBuffer.insert(rxBuffer.end(), readBuffer, readBuffer + bytesRead);

    // Attempt to parse one or more messages from rxBuffer
    while (true)
    {
        // We need at least the preamble + command + len + crc (for response preamble is 4)
        const int preambleSize = 4; // ResponseMessage preamble
        const size_t minHeader = preambleSize + 2 + 2 + 2; // preamble + cmd(2) + len(2) + crc(2)

        if (rxBuffer.size() < minHeader)
            break; // wait for more data

        // Ensure preamble is aligned at start; if not, discard until we find it
        const uint8_t preambleSeq[4] = {0xd5, 0xd5, 0xaa, 0x96};
        size_t firstPreamble = std::string::npos;
        for (size_t i = 0; i + preambleSize <= rxBuffer.size(); ++i)
        {
            if (std::memcmp(&rxBuffer[i], preambleSeq, preambleSize) == 0)
            {
                firstPreamble = i;
                break;
            }
        }

        if (firstPreamble == std::string::npos)
        {
            // no preamble in buffer yet - discard everything
            rxBuffer.clear();
            spdlog::debug("No preamble found in rxBuffer; discarding {} bytes", rxBuffer.size());
            break;
        }

        // Drop any leading garbage
        if (firstPreamble > 0)
        {
            rxBuffer.erase(rxBuffer.begin(), rxBuffer.begin() + firstPreamble);
            if (rxBuffer.size() < minHeader)
                break;
        }

        // Now we have preamble at start
        // Read payload length to figure full message size
        uint16_t payloadLen = static_cast<uint16_t>(rxBuffer[preambleSize + 2]) |
                              (static_cast<uint16_t>(rxBuffer[preambleSize + 3]) << 8);

        size_t fullMsgLen = preambleSize + 2 + 2 + payloadLen + 2;
        if (rxBuffer.size() < fullMsgLen)
        {
            // wait for rest
            break;
        }

        // We have enough bytes for a complete message. Try parsing.
        ResponseMessage resp;
        if (!resp.ParseMessage(rxBuffer.data(), fullMsgLen))
        {
            // Parse failed (likely CRC or corrupt). Drop first byte and retry.
            spdlog::warn("ParseMessage failed for candidate message (len={}), dropping one byte and resyncing", fullMsgLen);
            rxBuffer.erase(rxBuffer.begin());
            continue;
        }

        // Successful parse: emit a simple log point for measurements/events
        MessageType cmd = resp.GetMessageCommand();
        switch (cmd)
        {
            case MessageType::TempHumidityData:
            case MessageType::PirDataRaw:
            case MessageType::AmbientLightSensor:
            case MessageType::PirMotionEvent:
            case MessageType::ProximityEvent:
            case MessageType::ProximitySensorHighDetail:
            case MessageType::BackplateState:
            case MessageType::RawAdcData:
                spdlog::info("BackplateComms: Received measurement/event cmd=0x{:04x} size={}", static_cast<uint16_t>(cmd), resp.GetPayload().size());
                break;
            default:
                spdlog::info("BackplateComms: Received cmd=0x{:04x} size={}", static_cast<uint16_t>(cmd), resp.GetPayload().size());
                break;
        }

        // Invoke any subscribed callbacks (multi-subscriber std::function lists)
        const std::vector<uint8_t> &payload = resp.GetPayload();
        const uint8_t* payloadPtr = payload.size() ? payload.data() : nullptr;

        if (cmd == MessageType::TempHumidityData)
        {
            for (auto &cb : this->tempCallbacks)
            {
                if (cb) cb(payloadPtr, payload.size());
            }
        }

        if (cmd == MessageType::PirDataRaw || cmd == MessageType::PirMotionEvent)
        {
            for (auto &cb : this->pirCallbacks)
            {
                if (cb) cb(payloadPtr, payload.size());
            }
        }

        for (auto &cb : this->genericCallbacks)
        {
            if (cb) cb(static_cast<uint16_t>(cmd), payloadPtr, payload.size());
        }

        // Remove processed bytes
        rxBuffer.erase(rxBuffer.begin(), rxBuffer.begin() + fullMsgLen);
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