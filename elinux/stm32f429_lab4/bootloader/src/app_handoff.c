#include <stdint.h>

#include "arch/cortex-m4/barrier.h"
#include "arch/cortex-m4/scb_regs.h"
#include "common/include/memory_map.h"
#include "app_handoff.h"


__attribute__((naked, noreturn))
static void jump_to_app(uint32_t vector_base, uint32_t sp, uint32_t reset_vector)
{
    /* Relax compiler unused-parameter warnings */
    (void) vector_base;
    (void) sp;
    (void) reset_vector;

    __asm volatile(
        "cpsid i            \n"
        "movs r3, #0        \n"
        "msr control, r3    \n"
        "isb                \n"
        "msr msp, r1        \n"
        "dsb                \n"
        "isb                \n"
        "bx r2              \n"
    );
}


void app_handoff(uint32_t vector_table)
{
    const volatile uint32_t *vectors = (const volatile uint32_t *)(uintptr_t)vector_table;
    uint32_t initial_sp = vectors[0];
    uint32_t reset_vector = vectors[1];   /* Thumb address: bit 0 is set */

    /* Validate the stack pointer and reset vector */
    if ((initial_sp < SRAM_BASE) || (initial_sp > SRAM_BASE + SRAM_SIZE) || ((initial_sp & 7u) != 0u))
    {
        for (;;) { __asm volatile("nop"); }
        __builtin_unreachable();
    }

    /* Validate that the reset handler points into SDRAM. */
    uint32_t reset_address = reset_vector & ~1u;

    if (((reset_vector & 1u) == 0u) ||
        (reset_address < SDRAM_BASE) ||
        (reset_address >= SDRAM_BASE + SDRAM_SIZE)) {
        for (;;) { __asm volatile("nop"); }
    }
    
    /* Set the vector table offset register */
    SCB_VTOR = vector_table;
    
    /* Jump to the application */
    jump_to_app(vector_table, initial_sp, reset_vector);
}
