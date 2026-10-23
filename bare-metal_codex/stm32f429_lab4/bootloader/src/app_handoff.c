#include <stdint.h>

#include "arch/cortex-m4/barrier.h"
#include "arch/cortex-m4/scb_regs.h"
#include "arch/cortex-m4/systick_regs.h"
#include "soc/stm32f429/irq.h"
#include "common/include/memory_map.h"
#include "app_handoff.h"

#define NVIC_IRQS_PER_REGISTER 32u

#define NVIC_REGISTER_COUNT \
    ((IRQ_COUNT + NVIC_IRQS_PER_REGISTER - 1u) / NVIC_IRQS_PER_REGISTER)


static void bootloader_prepare_handoff(void)
{
    /* Stop SysTick and disable its interrupt generation. */
    SYSTICK_CTRL = 0u;
    SYSTICK_VAL = 0u;

    /* Clear a previously pending SysTick exception. */
    SCB_ICSR = SCB_ICSR_PENDSTCLR;

    /*
     * Disable and clear all external interrupts.
     *
     * STM32F429 has 91 external IRQs.
     * Each NVIC register controls 32 IRQs.
     * Therefore, we need 3 registers.
     */
    for (uint32_t i = 0; i < NVIC_REGISTER_COUNT; ++i)
    {
        NVIC_ICER(i) = 0xFFFFFFFFu;
        NVIC_ICPR(i) = 0xFFFFFFFFu;
    }

    dsb();
    isb();
}

__attribute__((naked, noreturn))
static void jump_to_app(uint32_t, uint32_t)
{
    __asm volatile(
        "movs r2, #0        \n"
        "msr control, r2    \n"
        "isb                \n"
        "msr msp, r0        \n"
        "dsb                \n"
        "isb                \n"
        "bx r1              \n"
    );
}


app_handoff_status_t app_handoff_validate(uint32_t vector_table)
{
    if (vector_table != SDRAM_BASE)
        return APP_HANDOFF_ERR_VECTOR_BASE;

    const volatile uint32_t *vectors = (const volatile uint32_t *)(uintptr_t)vector_table;
    uint32_t initial_sp = vectors[0];
    uint32_t reset_vector = vectors[1];   /* Thumb address: bit 0 is set */

    /* Validate the stack pointer and reset vector */
    if ((initial_sp <= SRAM_BASE) 
        || (initial_sp > SRAM_BASE + SRAM_SIZE)
        || ((initial_sp & 7u) != 0u))
    {
        return APP_HANDOFF_ERR_STACK;
    }

    /* Validate that the reset handler points into SDRAM. */
    uint32_t reset_address = reset_vector & ~1u;

    if (((reset_vector & 1u) == 0u) ||
        (reset_address < SDRAM_BASE) ||
        (reset_address >= SDRAM_BASE + SDRAM_SIZE)) {
        
        return APP_HANDOFF_ERR_RESET_VECTOR;
    }

    return APP_HANDOFF_OK;
}


void app_handoff(uint32_t vector_table)
{
    /* Mask the configurable interrupts */
    __asm volatile ("cpsid i" ::: "memory");

    const volatile uint32_t *vectors = (const volatile uint32_t *)(uintptr_t)vector_table;
    uint32_t initial_sp = vectors[0];
    uint32_t reset_vector = vectors[1];   /* Thumb address: bit 0 is set */

    /* Prepare for handoff to the application */
    bootloader_prepare_handoff();

    /* Set the vector table offset register */
    SCB_VTOR = vector_table;
    dsb();
    isb();
    
    /* Jump to the application */
    jump_to_app(initial_sp, reset_vector);
}
