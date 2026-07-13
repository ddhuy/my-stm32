#include <stdint.h>

#define RCC_AHB1ENR (*(volatile uint32_t *) 0x40023830)
#define GPIOG_MODER (*(volatile uint32_t *) 0x40021800)
#define GPIOG_ODR   (*(volatile uint32_t *) 0x40021814)

// Symbol defined in the linker script - its address is the top of the RAM
extern uint32_t _estack, // end of stack (top of RAM)
                _sidata, // start of initialized data in flash
                _sdata,  // start of initialized data in RAM
                _edata,  // end of initialized data in RAM
                _sbss,   // start of uninitialized data in RAM
                _ebss;   // end of uninitialized data in RAM

int blink_delay = 200000; // .data - initialized
int press_count = 0;      // .data - initialized
int bss_check;            // .bss - uninitialized

static void delay(volatile uint32_t n)
{
    while (n--) ;
}

int main(void)
{
    RCC_AHB1ENR |=  (1 << 6);       // enable GPIOG clock
    (void) RCC_AHB1ENR; // dummy read to ensure clock is enabled
    GPIOG_MODER &= ~(3 << (13 * 2)); // clear PG13 mode field
    GPIOG_MODER |=  (1 << (13 * 2)); // PG13 = general-purpose output

    while (1)
    {
        GPIOG_ODR ^= (1 << 13);
        delay(blink_delay);
    }
}

void reset_handler()
{
    // Copy initialized data from flash to RAM
    uint32_t *src = &_sidata;
    for (uint32_t *dst = &_sdata; dst < &_edata;)
        *dst++ = *src++;

    // Zero out the .bss section in RAM
    for (uint32_t *dst = &_sbss; dst < &_ebss;)
        *dst++ = 0;

    // Call the main function
    main();

    // trap if main ever returns
    while (1);
}

__attribute__((section(".isr_vector")))
const void *vector_table[] = 
{
    &_estack,       // word 0: initial stack pointer
    reset_handler   // word 1: reset vector
};