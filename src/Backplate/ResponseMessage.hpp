#pragma once

#include "Message.hpp"


class ResponseMessage : public Message {
public:
    ResponseMessage()
        : Message()
    {
    }

    ResponseMessage(MessageType cmd)
        : Message(cmd)
    {
    }

    virtual ~ResponseMessage()
    {
    }

    const uint8_t* GetPreamble() const override
    {
        static const uint8_t preamble[4] = {0xd5, 0xd5, 0xaa, 0x96};
        return preamble;
    }

    int GetPreambleSize() const override
    {
        return 4;
    }
};