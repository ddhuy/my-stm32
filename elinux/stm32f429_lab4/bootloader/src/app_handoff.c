#include <stdint.h>

#include "arch/cortex-m4/barrier.h"

#include "app_handoff.h"


void app_handoff(uint32_t vector_table)
{
    const volatile uint32_t *vectors = (const volatile uint32_t *)(uintptr_t)vector_table;
    uint32_t sp = vectors[0];
    uint32_t entry = vectors[1];   /* Thumb address: bit 0 is set */

    /* Make the copied image visible to instruction fetch. */
    dsb();
    isb();

    /*
     * After MSP changes, this function's own stack frame is gone, so the
     * switch and the jump happen in one asm block with no C in between.
     */
    __asm volatile(
        "msr msp, %0\n"
        "bx  %1\n"
        :
        : "r"(sp), "r"(entry)
        : "memory");

    __builtin_unreachable();
}
