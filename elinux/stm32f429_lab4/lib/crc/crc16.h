
#ifndef LIB_CRC16_H
#define LIB_CRC16_H

#include <stddef.h>
#include <stdint.h>

/*
 * CRC-16/XMODEM
 *
 * Polynomial: 0x1021
 * Initial:    0x0000
 * Reflected:  No
 * Final XOR:  0x0000
 */
uint16_t crc16_xmodem(const uint8_t *data, size_t length);

#endif
