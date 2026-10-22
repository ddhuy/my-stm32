#ifndef BOOT_XMODEM_IO_H
#define BOOT_XMODEM_IO_H

#include "lib/protocol/xmodem/xmodem.h"

/*
 * Return the board UART implementation of the
 * hardware-independent XMODEM transport interface.
 */
xmodem_io_t boot_xmodem_io_create(void);

/* Discard raw input until quiet_ms of silence, bounded by max_ms overall.
 * Requires the initialized polling timebase; owns COUNTFLAG while active.
 * Times are approximate. False means input is not safe for the command parser.
 */
bool boot_xmodem_io_drain(uint32_t quiet_ms, uint32_t max_ms);

#endif
