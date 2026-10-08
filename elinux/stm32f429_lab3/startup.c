#include <stdint.h>
extern uint32_t _estack, _sidata, _sdata, _edata, _sbss, _ebss;
extern int main(void);
extern void fault_handler(void);
void default_handler(void)
{
    for (;;)
        __asm volatile("bkpt #0");
}
void reset_handler(void)
{
    uint32_t *src = &_sidata;
    for (uint32_t *dst = &_sdata; dst < &_edata;)
        *dst++ = *src++;
    for (uint32_t *dst = &_sbss; dst < &_ebss;)
        *dst++ = 0;
    (void)main();
    for (;;)
    {
    }
}
/* 16 core vectors + 91 external IRQ vectors (STM32F429). */
__attribute__((section(".isr_vector"), used))
const uintptr_t vectors[16 + 91] = {
    [0] = (uintptr_t)&_estack,
    [1] = (uintptr_t)reset_handler,
    [2] = (uintptr_t)fault_handler,
    [3] = (uintptr_t)fault_handler,
    [4] = (uintptr_t)fault_handler,
    [5] = (uintptr_t)fault_handler,
    [6] = (uintptr_t)fault_handler,
    [11] = (uintptr_t)default_handler,
    [12] = (uintptr_t)default_handler,
    [14] = (uintptr_t)default_handler,
    [15] = (uintptr_t)default_handler,
    [16 ... 106] = (uintptr_t)default_handler};
