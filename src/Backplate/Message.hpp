#pragma once

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <vector>
#include "CRC-CITT.hpp"
#include "MessageType.hxx"


class Message {
public:
    Message(MessageType cmd)
        : commandId(cmd)
    {
    }

    Message()
        : commandId(MessageType::Null)
    {
    }

    virtual ~Message()
    {
    }

    virtual const uint8_t* GetPreamble() const = 0;
    virtual int GetPreambleSize() const = 0;

    const std::vector<uint8_t>& GetRawMessage();
    const MessageType GetMessageCommand() const { return commandId; }

    void SetPayload(const std::vector<uint8_t>& payload)
    {
        this->payload = payload;
    }

    const std::vector<uint8_t>& GetPayload() { return payload; }

    bool ParseMessage(const uint8_t* data, size_t length);

protected:
    void BuildMessage();
    void ClearBuffer() { buffer.clear(); }

protected:
    std::vector<uint8_t> buffer;
    MessageType commandId;
    std::vector<uint8_t> payload;

    static CRC_CITT CrcCalculator;
};
