#include <stdbool.h>

#include "soc/stm32f429/flash_regs.h"

#include "stm32_flash.h"

#define FLASH_ERASE_MAX_POLLS   100000000u
#define FLASH_PROGRAM_MAX_POLLS 1000000u

static bool stm32_flash_sector_valid(uint32_t sector)
{
    return sector < 12u;
}

static uint32_t stm32_flash_sector_erase_config(
    uint32_t sector)
{
    return FLASH_CR_SER |
           (sector << FLASH_CR_SNB_SHIFT) |
           FLASH_CR_PSIZE_X32;
}

/*
 * Unlock the Flash control interface.
 *
 * RM0090: Two unlock keys must be written to FLASH_KEYR
 * in the correct order.
 */
stm32_flash_status_t stm32_flash_unlock(void)
{
    if (FLASH_SR & FLASH_SR_BSY)
    {
        return STM32_FLASH_ERR_BUSY;
    }

    /* Already unlocked. */
    if ((FLASH_CR & FLASH_CR_LOCK) == 0u)
    {
        return STM32_FLASH_OK;
    }

    FLASH_KEYR = FLASH_KEY1;
    FLASH_KEYR = FLASH_KEY2;

    /* Verify that the LOCK bit was cleared. */
    if (FLASH_CR & FLASH_CR_LOCK)
    {
        return STM32_FLASH_ERR_LOCKED;
    }

    return STM32_FLASH_OK;
}

/*
 * Lock the Flash control interface.
 */
stm32_flash_status_t stm32_flash_lock(void)
{
    if (FLASH_SR & FLASH_SR_BSY)
    {
        return STM32_FLASH_ERR_BUSY;
    }

    FLASH_CR |= FLASH_CR_LOCK;

    if ((FLASH_CR & FLASH_CR_LOCK) == 0u)
    {
        return STM32_FLASH_ERR_LOCKED;
    }

    return STM32_FLASH_OK;
}

/*
 * Poll Flash status until the controller is idle.
 *
 * This function does not clear error flags.
 */
stm32_flash_status_t stm32_flash_wait_ready(
    uint32_t max_polls)
{
    for (uint32_t i = 0u; i < max_polls; ++i)
    {
        uint32_t sr = FLASH_SR;

        if ((sr & FLASH_SR_BSY) == 0u)
        {
            if (sr & FLASH_SR_ERROR_MASK)
            {
                return STM32_FLASH_ERR_OPERATION;
            }

            return STM32_FLASH_OK;
        }
    }

    return STM32_FLASH_ERR_TIMEOUT;
}

/*
 * Clear the EOP and operation-error flags.
 *
 * FLASH_SR flags are write-one-to-clear.
 */
stm32_flash_status_t stm32_flash_clear_status(void)
{
    if (FLASH_SR & FLASH_SR_BSY)
    {
        return STM32_FLASH_ERR_BUSY;
    }

    FLASH_SR = FLASH_SR_CLEAR_MASK;

    if (FLASH_SR & FLASH_SR_CLEAR_MASK)
    {
        return STM32_FLASH_ERR_OPERATION;
    }

    return STM32_FLASH_OK;
}


/*
 * Erase one Flash sector in Bank 1.
 *
 * Preconditions:
 * - Supply voltage supports 32-bit parallelism.
 * - Caller has authorized the target sector.
 *
 * The function manages Flash unlock/lock internally.
 */
stm32_flash_status_t stm32_flash_erase_sector(uint32_t sector)
{
    stm32_flash_status_t status;

    if (!stm32_flash_sector_valid(sector))
    {
        return STM32_FLASH_ERR_INVALID_SECTOR;
    }

    status = stm32_flash_wait_ready(FLASH_ERASE_MAX_POLLS);
    if (status != STM32_FLASH_OK)
    {
        return status;
    }

    status = stm32_flash_unlock();
    if (status != STM32_FLASH_OK)
    {
        return status;
    }

    status = stm32_flash_clear_status();
    if (status != STM32_FLASH_OK)
    {
        goto cleanup;
    }

    uint32_t cr = FLASH_CR;

    cr &= ~(FLASH_CR_PG |
            FLASH_CR_SER |
            FLASH_CR_MER |
            FLASH_CR_SNB_MASK |
            FLASH_CR_PSIZE_MASK);

    cr |= stm32_flash_sector_erase_config(sector);

    FLASH_CR = cr;

    /*
     * DESTRUCTIVE OPERATION:
     * Start erasing the selected sector.
     */
    FLASH_CR |= FLASH_CR_STRT;

    status = stm32_flash_wait_ready(FLASH_ERASE_MAX_POLLS);

    /*
     * Only clean up control bits when hardware is idle.
     * A poll-budget expiry does not prove an erase stopped.
     */
    if (FLASH_SR & FLASH_SR_BSY)
    {
        return STM32_FLASH_ERR_TIMEOUT;
    }

cleanup:
    FLASH_CR &= ~(FLASH_CR_SER |
                  FLASH_CR_SNB_MASK |
                  FLASH_CR_PSIZE_MASK);

    stm32_flash_status_t lock_status = stm32_flash_lock();

    if (status == STM32_FLASH_OK)
    {
        status = lock_status;
    }

    return status;
}

/*
 * Program one 32-bit word into internal Flash.
 *
 * Requirements:
 *   - Address is inside physical Flash.
 *   - Address is 4-byte aligned.
 *   - Target word is erased (0xFFFFFFFF).
 *   - VDD supports 32-bit programming.
 *
 * This is a hardware driver, so it does not enforce
 * bootloader-specific application boundaries.
 */
stm32_flash_status_t stm32_flash_program_word(
    uint32_t address,
    uint32_t value)
{
    stm32_flash_status_t status;

    /* Physical Flash range and alignment. */
    if (address < FLASH_BASE ||
        address >= FLASH_BASE + FLASH_SIZE ||
        (address & 3u) != 0u ||
        (FLASH_BASE + FLASH_SIZE - address) < 4u)
    {
        return STM32_FLASH_ERR_INVALID_ADDRESS;
    }

    status = stm32_flash_wait_ready(FLASH_PROGRAM_MAX_POLLS);
    if (status != STM32_FLASH_OK)
    {
        return status;
    }

    /*
     * This initial API requires an erased word.
     * Avoid attempting to reprogram an occupied word.
     */
    if (*(volatile const uint32_t *)(uintptr_t)address
        != 0xFFFFFFFFu)
    {
        return STM32_FLASH_ERR_OPERATION;
    }

    status = stm32_flash_unlock();
    if (status != STM32_FLASH_OK)
    {
        return status;
    }

    status = stm32_flash_clear_status();
    if (status != STM32_FLASH_OK)
    {
        goto cleanup;
    }

    uint32_t cr = FLASH_CR;

    /* Remove settings from previous Flash operations. */
    cr &= ~(FLASH_CR_PG |
            FLASH_CR_SER |
            FLASH_CR_MER |
            FLASH_CR_SNB_MASK |
            FLASH_CR_PSIZE_MASK);

    cr |= FLASH_CR_PSIZE_X32 | FLASH_CR_PG;

    FLASH_CR = cr;

    /*
     * Writing the destination word starts programming.
     */
    *(volatile uint32_t *)(uintptr_t)address = value;

    status = stm32_flash_wait_ready(FLASH_PROGRAM_MAX_POLLS);

    /*
     * If hardware is still busy, do not change
     * FLASH_CR or attempt to lock the controller.
     */
    if (FLASH_SR & FLASH_SR_BSY)
    {
        return STM32_FLASH_ERR_TIMEOUT;
    }

    /* Programming must be disabled before verification. */
    FLASH_CR &= ~FLASH_CR_PG;

    if (status == STM32_FLASH_OK)
    {
        uint32_t actual =
            *(volatile const uint32_t *)(uintptr_t)address;

        if (actual != value)
        {
            status = STM32_FLASH_ERR_VERIFY;
        }
    }

cleanup:
    /*
     * Only reached when the controller is idle.
     */
    FLASH_CR &= ~FLASH_CR_PG;

    stm32_flash_status_t lock_status = stm32_flash_lock();

    if (status == STM32_FLASH_OK)
    {
        status = lock_status;
    }

    return status;
}
