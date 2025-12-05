#pragma once

#include <vector>
#include <cstdint>
#include "ResponseMessage.hpp"

class MessageParser {
public:
    MessageParser();

    // Feed raw bytes into the parser. Returns any complete messages parsed
    // from the input (can be zero, one or more).
    std::vector<ResponseMessage> Feed(const uint8_t* data, size_t len);

private:
    std::vector<uint8_t> buffer_;
    static const uint8_t PREAMBLE[4];
};
