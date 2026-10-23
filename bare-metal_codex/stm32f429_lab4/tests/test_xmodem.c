#include <assert.h>
#include <stdio.h>
#include "lib/protocol/xmodem/xmodem.h"

int main(void)
{
    /* Fixed payload and CRC avoid using the implementation to build its oracle. */
    xmodem_packet_t valid = {
        .soh = XMODEM_SOH,
        .block = 1,
        .block_inv = 0xFE,
        .crc_hi = 0xE8,
        .crc_lo = 0x0A
    };
    
    for (size_t i = 0; i < XMODEM_BLOCK_SIZE; ++i)
        valid.data[i] = (uint8_t)i;
    
    assert(xmodem_packet_validate(NULL) == XMODEM_PACKET_ERR_FORMAT);
    assert(xmodem_packet_validate(&valid) == XMODEM_PACKET_OK);
    
    xmodem_packet_t p = valid;

    p.soh = 0x02; /* STX / XMODEM-1K is unsupported. */
    assert(xmodem_packet_validate(&p) == XMODEM_PACKET_ERR_FORMAT);

    p = valid;
    p.block_inv ^= 1;
    assert(xmodem_packet_validate(&p) == XMODEM_PACKET_ERR_BLOCK);

    for (size_t i = 0; i < XMODEM_BLOCK_SIZE; ++i)
    {
        p = valid;
        p.data[i] ^= 1;
        assert(xmodem_packet_validate(&p) == XMODEM_PACKET_ERR_CRC);
    }

    p = valid; p.crc_hi ^= 1;
    assert(xmodem_packet_validate(&p) == XMODEM_PACKET_ERR_CRC);

    p = valid; p.crc_lo ^= 1;
    assert(xmodem_packet_validate(&p) == XMODEM_PACKET_ERR_CRC);

    p = valid; p.crc_hi = valid.crc_lo; p.crc_lo = valid.crc_hi;
    assert(xmodem_packet_validate(&p) == XMODEM_PACKET_ERR_CRC);

    /* Integrity validation is independent of receiver sequence state. */
    for (unsigned block = 0; block < 256; ++block) {
        p = valid; p.block = (uint8_t)block; p.block_inv = (uint8_t)~block;
        assert(xmodem_packet_validate(&p) == XMODEM_PACKET_OK);
    }

    puts("XMODEM packet tests passed");
    return 0;
}
