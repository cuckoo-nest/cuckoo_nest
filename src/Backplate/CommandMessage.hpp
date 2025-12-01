#pragma once

#include "Message.hpp"


class CommandMessage : public Message {
public:
    CommandMessage(MessageType cmd)
        : Message(cmd)
    {
    }

    CommandMessage()
        : Message()
    {
    }

    virtual ~CommandMessage()
    {
    }

    const uint8_t* GetPreamble() const override
    {
        static const uint8_t preamble[3] = {0xd5, 0xaa, 0x96};
        return preamble;
    }

    int GetPreambleSize() const override
    {
        return 3;
    }
};