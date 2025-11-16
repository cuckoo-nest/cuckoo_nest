#pragma once

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <vector>
#include "CRC-CITT.hpp"

// typedef __attribute__((packed)) struct {
//     uint8_t preamble[3] = {0xd5, 0x5d, 0xc3};
//     uint16_t CommandId;
//     uint16_t PayloadLength;
//     uint8_t *Payload;
// }MessageHeader;


enum class MessageCommand : uint16_t {
    Null = 0x0000,
    Reset = 0x00FF,
    PeriodicStatusRequest = 0x0083,
    FetControl = 0x0082,
    FetPresenceAck = 0x008f,
    GetTfeId = 0x0090,
    GetTfeVersion = 0x0098,
    GetTfeBuildInfo = 0x0099,
    GetBackplateModelAndBslId = 0x009d,
    GetBslVersion = 0x009b,
    GetCoProcessorBslVersionInfo = 0x009c,
    GetHardwareVersionAndBackplateName = 0x009e,
    GetSerialNumber = 0x009f,
    SetPowerStealMode = 0x00C0,
    GetHistoricalDataBuffers = 0x00A2,
    AcknowledgeEndOfBuffers = 0x00A3,
    EndOfBuffersMessage = 0x002F,
    BufferedTemperatureData = 0x0022,
    PirSensorData = 0x0023,
    AmbientLightData = 0x000C,
    NearPir = 0x0007,
    Als = 0x000A,
    TemperatureLock = 0x00B1,

};

class Message {
public:
    Message(MessageCommand cmd)
        : commandId(cmd), payloadLength(0)
    {
    }

    Message()
        : commandId(MessageCommand::Null), payloadLength(0)
    {
    }

    virtual ~Message()
    {
    }

    const uint8_t Preamble[3] = {0xd5, 0x5d, 0xc3};
    const int PreambleSize = 3;

    const std::vector<uint8_t>& GetRawMessage();
    bool ParseMessage(const uint8_t* data, size_t length)
    {
        if (length < PreambleSize + 2 + 2 + 2) // Preamble + CommandId + PayloadLength + CRC
        {
            return false;
        }

        if (std::memcmp(data, Preamble, PreambleSize) != 0)
        {
            return false;
        }

        uint16_t receivedCrc = static_cast<uint16_t>(data[length - 2]) |
                      (static_cast<int16_t>(data[length - 1]) << 8);

        uint16_t calculatedCrc = CrcCalculator.Calculate(
                data + PreambleSize,
                length - PreambleSize - 2 // Exclude CRC bytes
            );
        
        if (receivedCrc != calculatedCrc)
        {
            return false;
        }

        return true;
    }

private:
    void BuildMessage();

private:
    std::vector<uint8_t> buffer;
    MessageCommand commandId;
    uint16_t payloadLength;

    static CRC_CITT CrcCalculator;
    
};