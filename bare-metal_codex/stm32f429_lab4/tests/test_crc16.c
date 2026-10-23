#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "lib/crc/crc16.h"

int main(void)
{
    /* Fixed CRC-16/XMODEM vectors, independently checked with crc_hqx(data, 0). */
    const uint8_t check[] = "123456789";
    uint8_t bytes[256];

    assert(crc16_xmodem(NULL, 0) == 0);
    assert(crc16_xmodem(check, 0) == 0);
    assert(crc16_xmodem(check, 9) == 0x31C3u);

    for (size_t i = 0; i < sizeof bytes; ++i)
        bytes[i] = (uint8_t)i;

    assert(crc16_xmodem(bytes, 128) == 0xE80Au);
    assert(crc16_xmodem(bytes, 256) == 0x7E55u);

    memset(bytes, 0, sizeof bytes);
    assert(crc16_xmodem(bytes, sizeof bytes) == 0);

    memset(bytes, 0xFF, sizeof bytes);
    assert(crc16_xmodem(bytes, 128) == 0xEDA9u);

    bytes[0] = 0x80;
    assert(crc16_xmodem(bytes, 1) == 0x9188u);

    /* The CRC is appended most-significant byte first. */
    const uint8_t codeword[] = {'1','2','3','4','5','6','7','8','9',0x31,0xC3};
    assert(crc16_xmodem(codeword, sizeof codeword) == 0);

    puts("CRC16 tests passed");

    return 0;
}
