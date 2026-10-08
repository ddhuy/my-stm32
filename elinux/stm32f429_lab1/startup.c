#include <stdint.h>
extern uint32_t _estack, _sidata, _sdata, _edata, _sbss, _ebss;
int main(void);
void Reset_Handler(void);
void Default_Handler(void) { while (1) {} }
void NMI_Handler(void) __attribute__((weak, alias("Default_Handler")));
void HardFault_Handler(void) __attribute__((weak, alias("Default_Handler")));
__attribute__((section(".isr_vector"), used))
const uintptr_t vectors[16] = {
 (uintptr_t)&_estack, (uintptr_t)Reset_Handler,
 (uintptr_t)NMI_Handler, (uintptr_t)HardFault_Handler,
 [4 ... 15] = (uintptr_t)Default_Handler
};
void Reset_Handler(void) {
 uint32_t *src = &_sidata;
 for (uint32_t *dst = &_sdata; dst < &_edata;) *dst++ = *src++;
 for (uint32_t *dst = &_sbss; dst < &_ebss;) *dst++ = 0;
 (void)main();
 while (1) {}
}
