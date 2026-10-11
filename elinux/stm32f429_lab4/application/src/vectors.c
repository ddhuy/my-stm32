#include <stdint.h>
#include "soc/stm32f429/irq.h"
#include "startup.h"

#define CORE_VECTOR_COUNT 16u
#define VECTOR_COUNT      (CORE_VECTOR_COUNT + IRQ_COUNT)

extern uint32_t __estack;

__attribute__((used, section(".isr_vector"), aligned(512)))
const uintptr_t vector_table[VECTOR_COUNT] = {
    [0] = (uintptr_t)&__estack,
    [1] = (uintptr_t)reset_handler,

    /* Cortex-M system exception entries */
    [2]  = (uintptr_t)default_handler,  /* NMI */
    [3]  = (uintptr_t)default_handler,  /* HardFault */
    [4]  = (uintptr_t)default_handler,  /* MemManage */
    [5]  = (uintptr_t)default_handler,  /* BusFault */
    [6]  = (uintptr_t)default_handler,  /* UsageFault */

    /* Entries 7-10 are reserved */
    [11] = (uintptr_t)default_handler,  /* SVCall */
    [12] = (uintptr_t)default_handler,  /* Debug Monitor */
    /* Entry 13 is reserved */
    [14] = (uintptr_t)default_handler,  /* PendSV */
    [15] = (uintptr_t)default_handler,  /* SysTick */

    /* External IRQ 0 .. IRQ_COUNT-1: temporary default handlers */
    [16 ... VECTOR_COUNT - 1] = (uintptr_t)default_handler
};