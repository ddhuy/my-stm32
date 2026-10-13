#include <stdint.h>

#include "arch/cortex-m4/scb_regs.h"
#include "soc/stm32f429/irq.h"
#include "startup.h"


#define CORE_VECTOR_COUNT 16u
#define VECTOR_COUNT      (CORE_VECTOR_COUNT + IRQ_COUNT)

extern uint32_t __estack;

extern uint32_t __sidata;
extern uint32_t __sdata;
extern uint32_t __edata;

extern uint32_t __sbss;
extern uint32_t __ebss;

extern int main(void);


/* Flash is aliased at 0x00000000 after reset, so VTOR = 0 already finds this. */
__attribute__((used, section(".isr_vector"), aligned(512)))
const uintptr_t vector_table[VECTOR_COUNT] = {
    [0] = (uintptr_t)&__estack,
    [1] = (uintptr_t)reset_handler,

    /* System exceptions 2..15 and IRQ 0 .. IRQ_COUNT-1: the bootloader enables none */
    [2 ... VECTOR_COUNT - 1] = (uintptr_t)default_handler,
};


void default_handler(void)
{
    while (1)
    {
    }
}

void reset_handler(void)
{
    // Set the vector table base address
    SCB_VTOR = (uint32_t)(uintptr_t)vector_table;
    
    // Initialize the .data
    for (uint32_t *src = &__sidata, *dst = &__sdata; dst < &__edata; )
    {
        *dst++ = *src++;
    }

    // Zero initialize the .bss section
    for (uint32_t *dst = &__sbss; dst < &__ebss; )
    {
        *dst++ = 0;
    }

    (void)main();

    // Trap if main ever returns
    for (;;) {
        __asm volatile("nop");
    }
}
