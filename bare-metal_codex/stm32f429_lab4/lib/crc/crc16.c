
#include "crc16.h"

uint16_t crc16_xmodem(const uint8_t *data, size_t length)
{
    uint16_t crc = 0u;

    for (size_t i = 0; i < length; ++i)
    {
        crc ^= (uint16_t)data[i] << 8;

        for (unsigned bit = 0; bit < 8u; ++bit)
        {
            if (crc & 0x8000u)
            {
                crc = (uint16_t)((crc << 1) ^ 0x1021u);
            }
            else
            {
                crc = (uint16_t)(crc << 1);
            }
        }
    }

    return crc;
}
