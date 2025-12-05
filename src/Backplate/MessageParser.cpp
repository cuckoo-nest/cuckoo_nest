#include "MessageParser.hpp"
#include <algorithm>
#include <cstring>

const uint8_t MessageParser::PREAMBLE[4] = {0xd5, 0xd5, 0xaa, 0x96};

MessageParser::MessageParser()
{
}

std::vector<ResponseMessage> MessageParser::Feed(const uint8_t* data, size_t len)
{
    std::vector<ResponseMessage> parsed;

    if (data != nullptr && len > 0) {
        buffer_.insert(buffer_.end(), data, data + len);
    }

    // Search loop: try to find and extract as many complete messages as possible
    while (true) {
        // Find start of preamble
        auto it = std::search(buffer_.begin(), buffer_.end(), PREAMBLE, PREAMBLE + 4);
        if (it == buffer_.end()) {
            // No preamble found. Keep up to 3 last bytes in case partial preamble
            if (buffer_.size() > 3) {
                std::vector<uint8_t> tail(buffer_.end() - 3, buffer_.end());
                buffer_.swap(tail);
            }
            break;
        }

        size_t idx = static_cast<size_t>(std::distance(buffer_.begin(), it));

        // If there's leading junk before the preamble, drop it
        if (idx > 0) {
            buffer_.erase(buffer_.begin(), buffer_.begin() + idx);
            // reset iterator to start
            it = buffer_.begin();
            idx = 0;
        }

        // Need minimum bytes for header: preamble(4) + cmd(2) + len(2) + crc(2)
        const size_t minHeader = 4 + 2 + 2 + 2;
        if (buffer_.size() < minHeader) {
            // wait for more data
            break;
        }

        // Extract payload length (little endian): at positions 6 and 7 (0-based)
        uint16_t payloadLen = static_cast<uint16_t>(buffer_[4 + 2]) |
                              (static_cast<uint16_t>(buffer_[4 + 3]) << 8);

        size_t totalMsgLen = 4 + 2 + 2 + payloadLen + 2; // preamble + cmd + len + payload + crc

        if (payloadLen > 65535) {
            // invalid length -> drop the preamble byte and continue
            buffer_.erase(buffer_.begin());
            continue;
        }

        if (buffer_.size() < totalMsgLen) {
            // Wait for full message
            break;
        }

        // We have a complete candidate message
        ResponseMessage msg;
        bool ok = msg.ParseMessage(buffer_.data(), totalMsgLen);
        if (ok) {
            parsed.push_back(msg);
            // Erase the consumed bytes
            buffer_.erase(buffer_.begin(), buffer_.begin() + totalMsgLen);
            // Continue to parse next messages
            continue;
        } else {
            // Parsing failed (likely CRC or malformed). Reset parser to initial
            // behaviour by discarding the first byte and continue searching
            buffer_.erase(buffer_.begin());
            continue;
        }
    }

    return parsed;
}
