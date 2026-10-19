#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "common/include/memory_map.h"
#include "board/stm32f429i-disc1/console.h"

#include "lib/protocol/xmodem/xmodem.h"

#include "boot_update.h"
#include "boot_xmodem_io.h"
#include "image_loader.h"

/*
 * SDRAM is used as a temporary firmware staging area.
 *
 * The bootloader executes from Flash and uses
 * internal SRAM for its stack and global variables.
 *
 * Ensure SDRAM has been initialized before calling.
 */
#define UPDATE_BUFFER_BASE  SDRAM_BASE
#define UPDATE_BUFFER_SIZE  SDRAM_SIZE

static const char *image_error_message(image_status_t status)
{
    switch (status)
    {
    case IMAGE_OK:               return "valid image";
    case IMAGE_ERR_HEADER:       return "invalid image buffer";
    case IMAGE_ERR_MAGIC:        return "invalid magic; send application.img";
    case IMAGE_ERR_LOAD_ADDRESS: return "unexpected load address";
    case IMAGE_ERR_SIZE:         return "invalid payload size";
    case IMAGE_ERR_FLASH_BOUNDS: return "image exceeds application flash capacity";
    case IMAGE_ERR_CRC:          return "payload CRC32 mismatch";
    case IMAGE_ERR_TRUNCATED:    return "incomplete image header or payload";
    default:                     return "unknown image validation error";
    }
}

bool boot_update_receive(void)
{
    uint8_t *buffer = (uint8_t *)UPDATE_BUFFER_BASE;
    size_t received_size = 0u;

    xmodem_io_t io = boot_xmodem_io_create();

    console_puts("\nXMODEM firmware update\n");
    console_puts("Waiting for sender...\n");

    /*
     * From here, XMODEM owns the UART.
     * Do not print console messages during transfer.
     */
    xmodem_status_t status = xmodem_receive(
        &io,
        buffer,
        UPDATE_BUFFER_SIZE,
        &received_size);

    if (status != XMODEM_OK)
    {
        console_puts("\nXMODEM transfer failed.\n");
        return false;
    }

    console_puts("\nXMODEM transfer completed.\n");

    /* Validate header + payload in place, excluding final-block padding. */
    image_status_t image_status = image_validate_buffer(buffer, received_size);
    if (image_status != IMAGE_OK)
    {
        console_puts("Image validation failed: ");
        console_puts(image_error_message(image_status));
        console_puts("\n");
        return false;
    }

    console_puts("Image validated and staged in SDRAM.\n");

    return true;
}