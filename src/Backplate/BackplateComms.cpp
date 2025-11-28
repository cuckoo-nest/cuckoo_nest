#include "BackplateComms.hpp"
#include "CommandMessage.hpp"
#include "ResponseMessage.hpp"
#include <unistd.h>
#include <iostream>

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
            std::cerr << "Burst stage timed out." << std::endl;
            break;
        }

        uint8_t readBuffer[256];
        size_t totalBytesRead = 0;
        totalBytesRead = SerialPort->Read(
            reinterpret_cast<char *>(readBuffer),
            sizeof(readBuffer));

        std::cout << "Read " << totalBytesRead << " bytes" << std::endl;

        if (totalBytesRead == 0)
        {
            std::cout << "No data read, continuing..." << std::endl;
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
        std::cerr << "Handshake Error: Did not receive 'BRK' signal." << std::endl;
        return false;
    }

    if (!fet_data_received)
    {
        std::cerr << "Handshake Error: Did not receive FET presence data." << std::endl;
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
        std::cerr << "Failed to get TfeVersion" << std::endl;
        return false;
    }

    if (!GetInfo(MessageType::GetTfeBuildInfo, MessageType::TfeBuildInfo))
    {
        std::cerr << "Failed to get TfeBuildInfo" << std::endl;
        return false;
    }

    if (!GetInfo(MessageType::GetBackplateModelAndBslId, MessageType::BackplateModelAndBslId))
    {
        std::cerr << "Failed to get BackplateModelAndBslId" << std::endl;
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
            std::cout << "GetInfo: ParseMessage failed"<< std::endl;
        }
        std::cout << "GetInfo: Received response for command "
                  << static_cast<uint16_t>(responseMsg.GetMessageCommand()) << std::endl;
        std::cout << "GetInfo: Checksum of payload: "
                  << static_cast<uint32_t>(responseMsg.GetPayload().size()) << " bytes" << std::endl;

        if (responseMsg.GetMessageCommand() == expectedResponse)
        {
            std::cout << "GetInfo: Received expected response for command "
                      << static_cast<uint16_t>(command) << std::endl;
            return true;
        }
    }

    std::cerr << "GetInfo Error: Did not receive expected response for command "
              << static_cast<uint16_t>(command) << std::endl;
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
                std::cout << "BackplateComms: Received measurement/event cmd=0x" << std::hex
                          << static_cast<uint16_t>(cmd) << std::dec << " size="
                          << resp.GetPayload().size() << std::endl;
                break;
            default:
                std::cout << "BackplateComms: Received cmd=0x" << std::hex
                          << static_cast<uint16_t>(cmd) << std::dec << " size="
                          << resp.GetPayload().size() << std::endl;
                break;
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