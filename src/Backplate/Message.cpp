#include "Message.hpp"

CRC_CITT Message::CrcCalculator;

const std::vector<uint8_t>& Message::GetRawMessage() 
{
    if (buffer.empty())
    {
        BuildMessage();
    }
    return buffer;
}

void Message::BuildMessage()
{
    const uint8_t* preamble = GetPreamble();
    int preambleSize = GetPreambleSize();
    
    for (int i = 0; i < preambleSize; ++i)
    {
        buffer.push_back(preamble[i]);
    }

    buffer.push_back(static_cast<uint8_t>(static_cast<uint16_t>(commandId) & 0x00FF));
    buffer.push_back(static_cast<uint8_t>((static_cast<uint16_t>(commandId) >> 8) & 0x00FF));

    buffer.push_back(static_cast<uint8_t>(payload.size() & 0x00FF));
    buffer.push_back(static_cast<uint8_t>((payload.size() >> 8) & 0x00FF));

    // Add payload if any
    for (const auto &b : payload)
    {
        buffer.push_back(b);
    }

    // Calculate CRC
    uint16_t crc = CrcCalculator.Calculate(
        buffer.data() + preambleSize, 
        buffer.size() - preambleSize
    );
    buffer.push_back(static_cast<uint8_t>(crc & 0x00FF));
    buffer.push_back(static_cast<uint8_t>((crc >> 8) & 0x00FF));
}

bool Message::ParseMessage(const uint8_t* data, size_t length)
{
    const uint8_t* preamble = GetPreamble();
    int preambleSize = GetPreambleSize();
    
    if (length < static_cast<size_t>(preambleSize + 2 + 2 + 2)) // Preamble + CommandId + PayloadLength + CRC
    {
        return false;
    }

    if (std::memcmp(data, preamble, preambleSize) != 0)
    {
        return false;
    }

    // At this point, extract the true length
    uint16_t payloadLength = static_cast<uint16_t>(data[preambleSize + 2]) |
                             (static_cast<uint16_t>(data[preambleSize + 3]) << 8);

    if (payloadLength > 0)
    {
        payload.resize(payloadLength);
        std::memcpy(payload.data(), data + preambleSize + 4, payloadLength);
    }

    uint16_t crcPosition = preambleSize + 2 + 2 + payloadLength;

    uint16_t receivedCrc = static_cast<uint16_t>(data[crcPosition]) |
                  (static_cast<int16_t>(data[crcPosition + 1]) << 8);

    uint16_t calculatedCrc = CrcCalculator.Calculate(
            data + preambleSize,
            crcPosition - preambleSize
        );
    
    if (receivedCrc != calculatedCrc)
    {
        return false;
    }

    commandId = static_cast<MessageType>(
        static_cast<uint16_t>(data[preambleSize]) |
        (static_cast<uint16_t>(data[preambleSize + 1]) << 8)
    );

    return true;
}
