#ifndef BOOT_UPDATE_H
#define BOOT_UPDATE_H

#include <stdbool.h>

/*
 * Receive application.img into SDRAM via XMODEM and validate its header,
 * received payload bounds, and CRC32. Returns true only after validation.
 *
 * Leaves the packaged image (including header) in staging memory.
 * Does not program internal Flash or boot the staged image.
 */
bool boot_update_receive(void);

#endif