#include "crc32.h"

uint32_t crc32_compute(const void *data, size_t size)
{
    const uint8_t *bytes = data;
    uint32_t crc = UINT32_C(0xFFFFFFFF);

    for (size_t i = 0; i < size; ++i) {
        crc ^= bytes[i];
        for (unsigned bit = 0; bit < 8u; ++bit) {
            if (crc & 1u)
                crc = (crc >> 1) ^ UINT32_C(0xEDB88320);
            else
                crc >>= 1;
        }
    }

    return crc ^ UINT32_C(0xFFFFFFFF);
}
