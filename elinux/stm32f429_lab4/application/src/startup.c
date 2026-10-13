#include <stdint.h>
#include "arch/cortex-m4/barrier.h"
#include "arch/cortex-m4/scb_regs.h"
#include "soc/stm32f429/tim_regs.h"
#include "startup.h"


extern uint32_t __estack;

extern uint32_t __sidata;
extern uint32_t __sdata;
extern uint32_t __edata;

extern uint32_t __sbss;
extern uint32_t __ebss;


extern int main(void);

volatile uint32_t tim2_irq_count = 0;


void tim2_irq_handler(void)
{
    if (TIM2_SR & TIM_SR_UIF) {
        TIM2_SR &= ~TIM_SR_UIF;
        tim2_irq_count++;
    }
}


void default_handler(void)
{
    while (1)
    {
    }
}


void reset_handler(void)
{
    /*
     * The image owns its vector table: point VTOR at it before anything can
     * fault, so the application works no matter which loader started it.
     */
    SCB_VTOR = (uint32_t)(uintptr_t)vector_table;
    dsb();
    isb();

    // Initialize the .data
    for (uint32_t* src = &__sidata, *dst = &__sdata; dst < &__edata; )
    {
        *dst++ = *src++;
    }

    // Zero initialize the .bss section
    for (uint32_t* dst = &__sbss; dst < &__ebss; )
    {
        *dst++ = 0;
    }

    // Call the main function after initializing the C runtime
    (void) main();

    // Trap if main ever returns
    while (1);
}
