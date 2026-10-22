#ifndef BOOT_INSTALL_H
#define BOOT_INSTALL_H

#include <stddef.h>
#include <stdint.h>

#include "drivers/flash/stm32_flash.h"
#include "boot_slot.h"


typedef enum
{
    BOOT_INSTALL_OK = 0,
    BOOT_INSTALL_ERR_ARGUMENT,
    BOOT_INSTALL_ERR_RANGE,
    BOOT_INSTALL_ERR_ERASE,
    BOOT_INSTALL_ERR_PROGRAM,
    BOOT_INSTALL_ERR_VERIFY
} boot_install_status_t;

typedef enum
{
    BOOT_INSTALL_STAGE_NONE = 0,
    BOOT_INSTALL_STAGE_VALIDATE,
    BOOT_INSTALL_STAGE_ERASE,
    BOOT_INSTALL_STAGE_PROGRAM,
    BOOT_INSTALL_STAGE_VERIFY
} boot_install_stage_t;

typedef struct
{
    boot_install_status_t status;
    boot_install_stage_t stage;

    stm32_flash_status_t flash_status;

    uint32_t flash_sr;
    uint32_t address;
} boot_install_result_t;

/*
 * Install an already-validated packaged image.
 *
 * result must be a valid pointer.
 *
 * On success:
 *   result->status == BOOT_INSTALL_OK
 *
 * On failure:
 *   result contains the operation stage,
 *   Flash driver status and diagnostic information.
 */
boot_install_status_t boot_install_image_detailed(
    const uint8_t *staged_image,
    size_t image_length,
    boot_install_result_t *result);

/*
 * Compatibility API.
 */
boot_install_status_t boot_install_image(
    const uint8_t *staged_image,
    size_t image_length);

/*
 * Install an already-validated image into a selected slot.
 *
 * The caller must authorize the destination slot.
 */
boot_install_status_t boot_install_image_to_slot_detailed(
    boot_slot_t slot,
    const uint8_t *staged_image,
    size_t image_length,
    boot_install_result_t *result);

#endif