#ifndef STM32_FLASH_H
#define STM32_FLASH_H

typedef enum
{
    STM32_FLASH_OK = 0,
    STM32_FLASH_ERR_BUSY,
    STM32_FLASH_ERR_LOCKED,
    STM32_FLASH_ERR_INVALID_SECTOR,
    STM32_FLASH_ERR_TIMEOUT,
    STM32_FLASH_ERR_OPERATION,
    STM32_FLASH_ERR_INVALID_ADDRESS,
    STM32_FLASH_ERR_VERIFY
} stm32_flash_status_t;

/*
 * Unlock the Flash control interface.
 * Does not erase or program Flash memory.
 */
stm32_flash_status_t stm32_flash_unlock(void);

/*
 * Lock the Flash control interface.
 */
stm32_flash_status_t stm32_flash_lock(void);

/*
 * Wait until the Flash controller is idle.
 *
 * max_polls is a bounded number of register checks,
 * not a timeout in milliseconds.
 */
stm32_flash_status_t stm32_flash_wait_ready(
    uint32_t max_polls);

/*
 * Clear Flash operation flags.
 * The controller must not be busy.
 */
stm32_flash_status_t stm32_flash_clear_status(void);

/*
 * Erase one Flash sector in Bank 1.
 *
 * Preconditions:
 * - Supply voltage supports 32-bit parallelism.
 * - Caller has authorized the target sector.
 *
 * The function manages Flash unlock/lock internally.
 */
stm32_flash_status_t stm32_flash_erase_sector(uint32_t sector);

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
    uint32_t value);
#endif
