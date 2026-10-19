#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "drivers/flash/stm32_flash.h"
#include "board/stm32f429i-disc1/console.h"
#include "common/include/memory_map.h"

#include "boot_install.h"

/*
 * Application-specific installation policy.
 *
 * The Flash driver itself does not know
 * about application Flash boundaries.
 */
static bool boot_install_range_valid(size_t image_length)
{
    return image_length != 0u &&
           image_length <= APP_UPDATE_FLASH_SIZE;
}

/*
 * Compare the installed image against the staged image.
 *
 * On failure, failed_address identifies the first
 * byte that differs.
 */
static bool boot_install_verify(
    const uint8_t *image,
    size_t image_length,
    uint32_t *failed_address)
{
    const volatile uint8_t *flash =
        (const volatile uint8_t *)(uintptr_t)APP_FLASH_BASE;

    for (size_t i = 0u; i < image_length; ++i)
    {
        if (flash[i] != image[i])
        {
            *failed_address = APP_FLASH_BASE + (uint32_t)i;
            return false;
        }
    }

    return true;
}

/*
 * Initialize the diagnostic result.
 */
static void boot_install_result_init(
    boot_install_result_t *result)
{
    result->status = BOOT_INSTALL_OK;
    result->stage = BOOT_INSTALL_STAGE_NONE;
    result->flash_status = STM32_FLASH_OK;
    result->flash_sr = 0u;
    result->address = 0u;
}

/*
 * Record a failed Flash driver operation.
 *
 * Capture FLASH_SR immediately, before any
 * subsequent operation can clear error flags.
 */
static boot_install_status_t boot_install_flash_error(
    boot_install_result_t *result,
    boot_install_status_t status,
    boot_install_stage_t stage,
    stm32_flash_status_t flash_status,
    uint32_t address)
{
    result->status = status;
    result->stage = stage;
    result->flash_status = flash_status;
    result->flash_sr = stm32_flash_status_register();
    result->address = address;

    return status;
}

/**
 * Return detail error message from the Install Stage 
 */
static const char *boot_install_stage_message(
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

boot_install_status_t boot_install_image_detailed(
    const uint8_t *staged_image,
    size_t image_length,
    boot_install_result_t *result)
{
    if (result == NULL)
    {
        return BOOT_INSTALL_ERR_ARGUMENT;
    }

    boot_install_result_init(result);

    /*
     * Preflight checks.
     *
     * This installer expects the caller to have
     * validated the image header and CRC32.
     */
    if (staged_image == NULL)
    {
        result->status = BOOT_INSTALL_ERR_ARGUMENT;
        result->stage = BOOT_INSTALL_STAGE_VALIDATE;
        return result->status;
    }

    if (!boot_install_range_valid(image_length))
    {
        result->status = BOOT_INSTALL_ERR_RANGE;
        result->stage = BOOT_INSTALL_STAGE_VALIDATE;
        return result->status;
    }

    /*
     * Erase the application sector.
     *
     * No bootloader sectors are selected.
     */
    result->stage = BOOT_INSTALL_STAGE_ERASE;
    result->address = APP_FLASH_BASE;

    stm32_flash_status_t flash_status =
        stm32_flash_erase_sector(APP_FLASH_FIRST_SECTOR);

    if (flash_status != STM32_FLASH_OK)
    {
        return boot_install_flash_error(
            result,
            BOOT_INSTALL_ERR_ERASE,
            BOOT_INSTALL_STAGE_ERASE,
            flash_status,
            APP_FLASH_BASE);
    }

    /*
     * Program the packaged image.
     *
     * The driver unlocks Flash once, programs
     * all words, and locks Flash afterward.
     */
    result->stage = BOOT_INSTALL_STAGE_PROGRAM;
    result->address = APP_FLASH_BASE;

    flash_status = stm32_flash_program(
        APP_FLASH_BASE,
        staged_image,
        image_length);

    if (flash_status != STM32_FLASH_OK)
    {
        /*
         * The current bulk API does not report
         * which word failed.
         *
         * APP_FLASH_BASE therefore identifies
         * the beginning of the programmed region.
         */
        return boot_install_flash_error(
            result,
            BOOT_INSTALL_ERR_PROGRAM,
            BOOT_INSTALL_STAGE_PROGRAM,
            flash_status,
            APP_FLASH_BASE);
    }

    /*
     * Verify the entire packaged image.
     */
    result->stage = BOOT_INSTALL_STAGE_VERIFY;

    uint32_t failed_address = 0u;

    if (!boot_install_verify(
            staged_image,
            image_length,
            &failed_address))
    {
        result->status = BOOT_INSTALL_ERR_VERIFY;
        result->address = failed_address;
        result->flash_sr = stm32_flash_status_register();

        return result->status;
    }

    boot_install_result_init(result);
    return BOOT_INSTALL_OK;
}

/*
 * Preserve the original public API.
 */
boot_install_status_t boot_install_image(
    const uint8_t *staged_image,
    size_t image_length)
{
    boot_install_result_t result;

    return boot_install_image_detailed(
        staged_image,
        image_length,
        &result);
}

/**
 * Print the detail of Install Stage.
 */
void boot_install_print_error(
    const boot_install_result_t *result)
{
    console_puts("\nFirmware installation failed!\n");

    console_puts("Stage        : ");
    console_puts(boot_install_stage_message(result->stage));
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
