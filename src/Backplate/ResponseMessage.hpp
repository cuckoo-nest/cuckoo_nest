#pragma once

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <vector>
#include "CRC-CITT.hpp"
#include "MessageType.hxx"


class ResponseMessage {
public:
    ResponseMessage()
        : commandId(MessageType::Null)
    {
    }

    ResponseMessage(MessageType cmd)
        : commandId(cmd)
    {
    }

    virtual ~ResponseMessage()
    {
    }

    const uint8_t Preamble[4] = {0xd5, 0xd5, 0xaa, 0x96};
    const int PreambleSize = sizeof(Preamble) / sizeof(Preamble[0]);

    void SetPayload(const std::vector<uint8_t>& payload)
    {
        this->payload = payload;
    }

    const std::vector<uint8_t>& GetPayload() {return payload;}

    const std::vector<uint8_t>& GetRawMessage();
    const MessageType GetMessageCommand() const { return commandId; }
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

        commandId = static_cast<MessageType>(
            static_cast<uint16_t>(data[PreambleSize]) |
            (static_cast<uint16_t>(data[PreambleSize + 1]) << 8)
        );

        uint16_t payloadLength = static_cast<uint16_t>(data[PreambleSize + 2]) |
                                 (static_cast<uint16_t>(data[PreambleSize + 3]) << 8);
        if (payloadLength > 0)
        {
            payload.resize(payloadLength);
            std::memcpy(payload.data(), data + PreambleSize + 4, payloadLength);
        }

        return true;
    }

private:
    void BuildMessage();

private:
    std::vector<uint8_t> buffer;
    MessageType commandId;
    std::vector<uint8_t> payload;

    static CRC_CITT CrcCalculator;
    
};