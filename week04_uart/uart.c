#include <stdint.h>

#define RCC_AHB1ENR (*(volatile uint32_t *) 0x40023830)
#define RCC_APB2ENR (*(volatile uint32_t *) 0x40023844)

#define GPIOA_MODER (*(volatile uint32_t *) 0x40020000)
#define GPIOA_AFRH  (*(volatile uint32_t *) 0x40020024)

#define GPIOG_MODER (*(volatile uint32_t *) 0x40021800)
#define GPIOG_ODR   (*(volatile uint32_t *) 0x40021814)

#define USART1_SR   (*(volatile uint32_t *) 0x40011000)
#define USART1_DR   (*(volatile uint32_t *) 0x40011004)
#define USART1_BRR  (*(volatile uint32_t *) 0x40011008)
#define USART1_CR1  (*(volatile uint32_t *) 0x4001100C)


// Symbol defined in the linker script - its address is the top of the RAM
extern uint32_t _estack, // end of stack (top of RAM)
                _sidata, // start of initialized data in flash
                _sdata,  // start of initialized data in RAM
                _edata,  // end of initialized data in RAM
                _sbss,   // start of uninitialized data in RAM
                _ebss;   // end of uninitialized data in RAM

int blink_delay = 4000000; // .data - initialized
int press_count = 0;      // .data - initialized
int bss_check;            // .bss - uninitialized


static char itoc(uint8_t i)
{
    if (0 <= i && i <= 9)
        return (0x30 + i);
    else if (0xA <= i && i <= 0xF)
        return (0x41 + (i - 0xA));
    else
        return 0x00;
}


static void delay(volatile uint32_t n)
{
    while (n--) ;
}


static void uart_init(void)
{
    RCC_AHB1ENR |= (1 << 0);       // enable GPIOA clock
    (void) RCC_AHB1ENR; // dummy read to ensure clock is enabled
    RCC_APB2ENR |= (1 << 4);       // enable USART1 clock
    (void) RCC_APB2ENR; // dummy read to ensure clock is enabled

    GPIOA_MODER &= ~((3 << (9 * 2)) | (3 << (10 * 2))); // clear PA9 & PA10 mode field
    GPIOA_MODER |=  ((2 << (9 * 2)) | (2 << (10 * 2))); // PA9 & PA10 = alternate function

    GPIOA_AFRH  &= ~((0xF << 4) | (0xF << 8)); // clear PA9 & PA10 AF field
    GPIOA_AFRH  |=  ((7 << 4) | (7 << 8));     // PA9 & PA10 = AF7 (USART1)

    // RCC_APB2ENR |= (1 << 4);        // enable USART1 clock
    // (void) RCC_APB2ENR; // dummy read to ensure clock is enabled

    USART1_BRR = 16000000 / 115200; // baud rate = SYSCLK / baud
    USART1_CR1 = (1 << 2) | (1 << 3) | (1 << 13); // RE=1, TE=1, UE=1
}

static void uart_putc(char c)
{
    while (!(USART1_SR & (1 << 7))) ; // wait until TXE is set
    USART1_DR = c;
}


static void uart_puts(const char *s)
{
    while (*s) {
        if (*s == '\n')
            uart_putc('\r');
        uart_putc(*s++);
    }
}


static void uart_putdec(uint32_t n)
{
    char s[10] = { 0 };
    uint8_t i = 0;

    do {
        s[i++] = 0x30 + (n % 10);
        n = n / 10;
    } while (n);

    while (i--)
        uart_putc(s[i]);
}


static void uart_puthex(uint32_t n)
{
    char s[10] = { 0 };
    
    uint8_t *p = (uint8_t *) &n;

    // Little-endian
    s[0] = itoc((p[3] >> 4) & 0x0F); s[1] = itoc(p[3] & 0x0F);
    s[2] = itoc((p[2] >> 4) & 0x0F); s[3] = itoc(p[2] & 0x0F);
    s[4] = itoc((p[1] >> 4) & 0x0F); s[5] = itoc(p[1] & 0x0F);
    s[6] = itoc((p[0] >> 4) & 0x0F); s[7] = itoc(p[0] & 0x0F);

    uart_puts(s);
}


int main(void)
{
    RCC_AHB1ENR |=  (1 << 6);       // enable GPIOG clock
    (void) RCC_AHB1ENR; // dummy read to ensure clock is enabled
    GPIOG_MODER &= ~(3 << (13 * 2)); // clear PG13 mode field
    GPIOG_MODER |=  (1 << (13 * 2)); // PG13 = general-purpose output

    uart_init();

    uart_puts("\n=== week04 bare-metal boot ===\n");
    uart_puts("SYSCLK: ");  uart_putdec(16000000);  uart_puts(" Hz (HSI)\n");
    uart_puts("blink_delay @ ");  uart_puthex((uint32_t) &blink_delay);
    uart_puts(" = ");  uart_puthex((uint32_t) blink_delay); uart_puts("\n");

    while (1)
    {
        GPIOG_ODR ^= (1 << 13);

        if (GPIOG_ODR & (1 << 13))
            uart_puts("Blink ON\n");
        else
            uart_puts("Blink OFF\n");

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