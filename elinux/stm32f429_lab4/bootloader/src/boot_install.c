
#include <stddef.h>
#include <stdint.h>

#include "drivers/flash/stm32_flash.h"
#include "common/include/memory_map.h"

#include "boot_install.h"

#define FLASH_WORD_SIZE 4u

/*
 * Verify that the packaged image fits entirely
 * inside the permitted application update region.
 */
static int boot_install_range_valid(size_t image_length)
{
    if (image_length == 0u)
    {
        return 0;
    }

    return image_length <= APP_UPDATE_FLASH_SIZE;
}

/*
 * Compare programmed Flash against the staged
 * image, byte by byte.
 */
static int boot_install_verify(
    const uint8_t *image,
    size_t image_length)
{
    const volatile uint8_t *flash =
        (const volatile uint8_t *)(uintptr_t)APP_FLASH_BASE;

    for (size_t i = 0u; i < image_length; ++i)
    {
        if (flash[i] != image[i])
        {
            return 0;
        }
    }

    return 1;
}

boot_install_status_t boot_install_image(
    const uint8_t *staged_image,
    size_t image_length)
{
    if (staged_image == NULL)
    {
        return BOOT_INSTALL_ERR_ARGUMENT;
    }

    if (!boot_install_range_valid(image_length))
    {
        return BOOT_INSTALL_ERR_RANGE;
    }

    /*
     * This initial installer supports only sector 5.
     * The bootloader's sectors 0-4 are never selected.
     */
    stm32_flash_status_t flash_status =
        stm32_flash_erase_sector(APP_FLASH_FIRST_SECTOR);

    if (flash_status != STM32_FLASH_OK)
    {
        return BOOT_INSTALL_ERR_ERASE;
    }

    /*
     * Program the complete packaged image:
     *   [16-byte header][application payload]
     */
    flash_status = stm32_flash_program(
        APP_FLASH_BASE,
        staged_image,
        image_length);

    if (flash_status != STM32_FLASH_OK)
    {
        return BOOT_INSTALL_ERR_PROGRAM;
    }

    /*
     * Verify the actual image bytes.
     * Final-word padding is excluded.
     */
    if (!boot_install_verify(staged_image, image_length))
    {
        return BOOT_INSTALL_ERR_VERIFY;
    }

    return BOOT_INSTALL_OK;
}
