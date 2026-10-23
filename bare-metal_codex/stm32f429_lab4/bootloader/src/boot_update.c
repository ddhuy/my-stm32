#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "common/include/memory_map.h"
#include "board/stm32f429i-disc1/console.h"

#include "lib/protocol/xmodem/xmodem.h"

#include "boot_install.h"
#include "boot_metadata.h"
#include "boot_slot.h"
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
#define UPDATE_DRAIN_QUIET_MS  1000u
#define UPDATE_DRAIN_LIMIT_MS  5000u

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


/**
 * Return detail error message from the Install Stage 
 */
static const char *boot_update_stage_message(
    boot_install_stage_t stage)
{
    switch (stage)
    {
    case BOOT_INSTALL_STAGE_VALIDATE:
        return "Image / range validation";

    case BOOT_INSTALL_STAGE_ERASE:
        return "Flash erase";

    case BOOT_INSTALL_STAGE_PROGRAM:
        return "Flash programming";

    case BOOT_INSTALL_STAGE_VERIFY:
        return "Flash readback verification";

    default:
        return "Unknown";
    }
}


/**
 * Print the detail of Install Stage.
 */
static void boot_update_print_error(
    const boot_install_result_t *result)
{
    console_puts("\nFirmware installation failed!\n");

    console_puts("Stage        : ");
    console_puts(boot_update_stage_message(result->stage));
    console_puts("\n");

    console_puts("Install error: ");
    console_puthex((uint32_t)result->status);
    console_puts("\n");

    console_puts("Flash status : ");
    console_puthex((uint32_t)result->flash_status);
    console_puts("\n");

    console_puts("FLASH_SR     : ");
    console_puthex(result->flash_sr);
    console_puts("\n");

    console_puts("Address      : ");
    console_puthex(result->address);
    console_puts("\n");

    console_puts("Returning to recovery console.\n");
}

/*
 * Choose the inactive firmware slot.
 *
 * If a bootable image exists, preserve it and
 * install the replacement into the other slot.
 *
 * If neither slot is valid, use Slot B as our
 * recovery installation destination.
 */
static boot_slot_t boot_update_target_slot(void)
{
    boot_slot_t active_slot;

    if (boot_select_slot(&active_slot))
    {
        return (active_slot == BOOT_SLOT_A)
            ? BOOT_SLOT_B
            : BOOT_SLOT_A;
    }

    return BOOT_SLOT_B;
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
    xmodem_status_t xmodem_status = xmodem_receive(
        &io,
        buffer,
        UPDATE_BUFFER_SIZE,
        &received_size);

    if (xmodem_status != XMODEM_OK)
    {
        /* Finish transmitting any cancellation before waiting for silence. */
        console_flush();
        if (!boot_xmodem_io_drain(UPDATE_DRAIN_QUIET_MS, UPDATE_DRAIN_LIMIT_MS))
        {
            console_puts("\nXMODEM input did not settle. Stop the sender and reset the board.\n");
            console_flush();
            /* Never interpret an ongoing binary transfer as console commands. */
            for (;;)
            {
            }
        }
        console_puts("\nXMODEM transfer failed.\n");
        return false;
    }

    console_puts("\nXMODEM transfer completed.\n");
    
    /*
    * Validate the complete staged image before
    * interpreting its header or touching Flash.
    */
    image_status_t image_status =
        image_validate_buffer(buffer, received_size);

    if (image_status != IMAGE_OK)
    {
        console_puts("Image validation failed: ");
        console_puts(image_error_message(image_status));
        console_puts("\n");
        return false;
    }

    console_puts("Image validated and staged in SDRAM.\n");

    /*
    * Safe to access the header after validation.
    */
    const app_image_header_t *header =
        (const app_image_header_t *)buffer;

    size_t actual_image_length =
        sizeof(app_image_header_t) + header->image_size;

    /*
     * Choose the slot that is not currently booted.
     * If neither slot is bootable, use Slot B for recovery.
     */
    boot_slot_t target_slot = boot_update_target_slot();

    boot_slot_info_t slot_info;

    if (!boot_slot_get_info(target_slot, &slot_info))
    {
        console_puts("Invalid destination slot.\n");
        return false;
    }

    /*
     * Reject oversized images before erasing Flash.
     */
    if (actual_image_length > slot_info.size)
    {
        console_puts("Firmware exceeds slot capacity.\n");
        return false;
    }

    /*
     * Reject incomplete images.
     */
    if (actual_image_length > received_size)
    {
        console_puts("Incomplete firmware image.\n");
        return false;
    }

    console_puts("Installing firmware into Slot ");
    console_puts(target_slot == BOOT_SLOT_A ? "A\n" : "B\n");

    boot_install_result_t install_result;

    boot_install_status_t install_status =
        boot_install_image_to_slot_detailed(
            target_slot,
            buffer,
            actual_image_length,
            &install_result);

    console_puts("Flash destination: ");
    console_puthex(slot_info.base);
    console_puts("\n");

    if (install_status != BOOT_INSTALL_OK)
    {
        boot_update_print_error(&install_result);
        return false;
    }

    /*
     * Revalidate the installed packaged image.
     */
    image_status_t installed_status =
        image_validate_slot(target_slot);

    if (installed_status != IMAGE_OK)
    {
        console_puts("Installed image validation failed: ");
        console_puts(image_error_message(installed_status));
        console_puts("\n");
        return false;
    }

    console_puts("Flash installation verified.\n");
    console_puts("Boot selection has not changed.\n");

    return true;
}
