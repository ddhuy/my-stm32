#include <stdint.h>
#define REG32(a) (*(volatile uint32_t *)(a))
#define RCC_AHB1ENR REG32(0x40023830UL)
#define GPIOG_MODER REG32(0x40021800UL)
#define GPIOG_BSRR REG32(0x40021818UL)
#define LED (1UL << 13)
static void delay(void) {
 for (volatile uint32_t i = 0; i < 800000; ++i) __asm volatile("nop");
}
int main(void) {
 RCC_AHB1ENR |= (1UL << 6);
 (void)RCC_AHB1ENR;
 GPIOG_MODER = (GPIOG_MODER & ~(3UL << 26)) | (1UL << 26);
 while (1) {
   GPIOG_BSRR = LED;
   delay();
   GPIOG_BSRR = LED << 16;
   delay();
 }
}
