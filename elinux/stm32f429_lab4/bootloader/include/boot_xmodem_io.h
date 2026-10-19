#ifndef BOOT_XMODEM_IO_H
#define BOOT_XMODEM_IO_H

#include "lib/protocol/xmodem/xmodem.h"

/*
 * Return the board UART implementation of the
 * hardware-independent XMODEM transport interface.
 */
xmodem_io_t boot_xmodem_io_create(void);

#endif