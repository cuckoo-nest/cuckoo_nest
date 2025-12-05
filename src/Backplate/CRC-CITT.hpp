#pragma once

#include <cstdint>
#include <cstddef>

class CRC_CITT
{
public:
    CRC_CITT()
    {
        const uint16_t poly = 0x1021;
        for (int i = 0; i < 256; ++i)
        {
            uint16_t r = (uint16_t)i << 8;
            for (int j = 0; j < 8; ++j)
            {
                if (r & 0x8000)
                    r = (r << 1) ^ poly;
                else
                    r <<= 1;
            }
            crctab[i] = r;
        }
    }

    uint16_t Calculate(const uint8_t *data, size_t length)
    {
        uint16_t crc = 0;
        for (size_t i = 0; i < length; ++i)
        {
            crc = (crc << 8) ^ crctab[((crc >> 8) ^ data[i]) & 0xff];
        }
        return crc;
    }

private:
    uint16_t crctab[0x100];
};