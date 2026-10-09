#include <stdint.h>
#include "startup.h"


extern uint32_t __estack;

extern uint32_t __sidata;
extern uint32_t __sdata;
extern uint32_t __edata;

extern uint32_t __sbss;
extern uint32_t __ebss;


extern int main(void);


void default_handler(void)
{
    while (1)
    {
    }
}


void reset_handler(void)
{
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
