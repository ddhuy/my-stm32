#ifndef IMAGE_FORMAT_H
#define IMAGE_FORMAT_H

/*
 * Packaged application image, as written by scripts/package_image.py:
 * a 16-byte header followed by the raw application binary.
 * Where the image lives and where it loads is in memory_map.h.
 */

#include "arch/cortex-m4/const.h"

#define APP_IMAGE_MAGIC     _AC(0x00070013, u)

#ifndef __ASSEMBLER__

#include <stdint.h>

typedef struct {
    uint32_t magic;        /* APP_IMAGE_MAGIC */
    uint32_t image_size;   /* Size of the payload after this header */
    uint32_t load_address; /* Where the bootloader copies the payload */
    uint32_t crc;          /* Reserved for a CRC32 of the payload (written as 0) */
} app_image_header_t;

_Static_assert(sizeof(app_image_header_t) == 16,
               "Application image header must be 16 bytes");

#endif /* __ASSEMBLER__ */

#endif
