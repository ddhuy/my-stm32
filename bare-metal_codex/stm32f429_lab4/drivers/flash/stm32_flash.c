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
 * Validate the complete word-addressed destination, including any
 * 0xFF padding required by an incomplete final word.
 */
static bool stm32_flash_program_range_valid(uint32_t address, size_t length)
{
    const uint32_t flash_end = FLASH_BASE + FLASH_SIZE;

    if (address < FLASH_BASE || address >= flash_end ||
        (address & 3u) != 0u || length == 0u)
    {
        return false;
    }

    size_t available = (size_t)(flash_end - address);
    if (length > available)
    {
        return false;
    }

    /* The last 32-bit Flash write must fit even after padding. */
    return (length / 4u + ((length % 4u) != 0u)) <= (available / 4u);
}

/* Prefer the operation failure to a secondary cleanup failure. */
static stm32_flash_status_t stm32_flash_finish_program(
    stm32_flash_status_t status)
{
    if (FLASH_SR & FLASH_SR_BSY)
    {
        /* Caller must reset or explicitly recover after hardware is idle. */
        return STM32_FLASH_ERR_TIMEOUT;
    }

    FLASH_CR &= ~FLASH_CR_PG;
    stm32_flash_status_t lock_status = stm32_flash_lock();
    return status == STM32_FLASH_OK ? lock_status : status;
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
    if (!stm32_flash_program_range_valid(address, 4u))
    {
        return STM32_FLASH_ERR_INVALID_ADDRESS;
    }

    stm32_flash_status_t status = stm32_flash_unlock();
    if (status != STM32_FLASH_OK)
    {
        return status;
    }

    status = stm32_flash_program_word_unlocked(address, value);
    return stm32_flash_finish_program(status);
}

/*
 * Program one word while Flash is already unlocked.
 * Leaves Flash unlocked, but clears PG once the controller is idle.
 * Does not modify controller configuration if still busy on timeout.
 */
stm32_flash_status_t stm32_flash_program_word_unlocked(
    uint32_t address, uint32_t value)
{
    if (!stm32_flash_program_range_valid(address, 4u))
    {
        return STM32_FLASH_ERR_INVALID_ADDRESS;
    }

    if (FLASH_CR & FLASH_CR_LOCK)
    {
        return STM32_FLASH_ERR_LOCKED;
    }

    stm32_flash_status_t status =
        stm32_flash_wait_ready(FLASH_PROGRAM_MAX_POLLS);
    if (status != STM32_FLASH_OK)
    {
        return status;
    }

    if (*(volatile const uint32_t *)(uintptr_t)address != 0xFFFFFFFFu)
    {
        return STM32_FLASH_ERR_OPERATION;
    }

    status = stm32_flash_clear_status();
    if (status != STM32_FLASH_OK)
    {
        return status;
    }

    /* No programming pulse is required to retain erased 0xFFFFFFFF. */
    if (value == 0xFFFFFFFFu)
    {
        return STM32_FLASH_OK;
    }

    uint32_t cr = FLASH_CR;
    cr &= ~(FLASH_CR_PG | FLASH_CR_SER | FLASH_CR_MER |
            FLASH_CR_SNB_MASK | FLASH_CR_PSIZE_MASK);
    FLASH_CR = cr | FLASH_CR_PSIZE_X32 | FLASH_CR_PG;

    *(volatile uint32_t *)(uintptr_t)address = value;

    status = stm32_flash_wait_ready(FLASH_PROGRAM_MAX_POLLS);

    /* After a timeout, the hardware may still be busy. Do not touch CR. */
    if (FLASH_SR & FLASH_SR_BSY)
    {
        return STM32_FLASH_ERR_TIMEOUT;
    }

    FLASH_CR &= ~FLASH_CR_PG;

    if (status == STM32_FLASH_OK &&
        *(volatile const uint32_t *)(uintptr_t)address != value)
    {
        status = STM32_FLASH_ERR_VERIFY;
    }

    return status;
}

stm32_flash_status_t stm32_flash_program(
    uint32_t address, const uint8_t *data, size_t length)
{
    if (data == NULL || !stm32_flash_program_range_valid(address, length))
    {
        return STM32_FLASH_ERR_INVALID_ADDRESS;
    }

    stm32_flash_status_t status = stm32_flash_unlock();
    if (status != STM32_FLASH_OK)
    {
        return status;
    }

    for (size_t offset = 0u; offset < length; offset += 4u)
    {
        uint32_t word = 0xFFFFFFFFu;
        for (size_t i = 0u; i < 4u && i < length - offset; ++i)
        {
            uint32_t mask = 0xFFu << (8u * i);
            word = (word & ~mask) | ((uint32_t)data[offset + i] << (8u * i));
        }

        status = stm32_flash_program_word_unlocked(
            address + (uint32_t)offset, word);
        if (status != STM32_FLASH_OK)
        {
            break;
        }
    }

    return stm32_flash_finish_program(status);
}

uint32_t stm32_flash_status_register(void)
{
    return FLASH_SR;
}