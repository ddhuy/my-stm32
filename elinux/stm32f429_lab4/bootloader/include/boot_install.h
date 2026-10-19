#ifndef BOOT_INSTALL_H
#define BOOT_INSTALL_H

#include <stddef.h>
#include <stdint.h>

typedef enum
{
    BOOT_INSTALL_OK = 0,
    BOOT_INSTALL_ERR_ARGUMENT,
    BOOT_INSTALL_ERR_RANGE,
    BOOT_INSTALL_ERR_ERASE,
    BOOT_INSTALL_ERR_PROGRAM,
    BOOT_INSTALL_ERR_VERIFY
} boot_install_status_t;

/*
 * Install an already-validated packaged image.
 *
 * Preconditions:
 * - The caller has validated the image header and CRC32.
 * - staged_image points to readable memory outside
 *   the target Flash sector.
 * - image_length is the actual packaged-image length,
 *   excluding XMODEM padding.
 *
 * Installs the image at APP_FLASH_BASE.
 */
boot_install_status_t boot_install_image(
    const uint8_t *staged_image,
    size_t image_length);

#endif