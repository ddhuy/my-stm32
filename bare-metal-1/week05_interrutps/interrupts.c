#include <stdint.h>

/* ---------------- RCC (RM0090 ch. 6) ---------------- */
#define RCC_AHB1ENR    (*(volatile uint32_t *)0x40023830)
#define RCC_APB2ENR    (*(volatile uint32_t *)0x40023844)

/* ---------------- GPIO (RM0090 ch. 8) ---------------- */
#define GPIOA_MODER    (*(volatile uint32_t *)0x40020000)
#define GPIOA_AFRH     (*(volatile uint32_t *)0x40020024)  /* AFR[1]: pins 8..15 */
#define GPIOG_MODER    (*(volatile uint32_t *)0x40021800)
#define GPIOG_ODR      (*(volatile uint32_t *)0x40021814)

/* ---------------- USART1 (RM0090 ch. 30) ---------------- */
#define USART1_SR      (*(volatile uint32_t *)0x40011000)
#define USART1_DR      (*(volatile uint32_t *)0x40011004)
#define USART1_BRR     (*(volatile uint32_t *)0x40011008)
#define USART1_CR1     (*(volatile uint32_t *)0x4001100C)

/* ---------------- SYSCFG + EXTI (RM0090 ch. 9, 12) ---------------- */
#define SYSCFG_EXTICR1 (*(volatile uint32_t *)0x40013808)
#define EXTI_IMR       (*(volatile uint32_t *)0x40013C00)
#define EXTI_RTSR      (*(volatile uint32_t *)0x40013C08)
#define EXTI_PR        (*(volatile uint32_t *)0x40013C14)

/* ------------- Cortex-M4 core: SysTick + NVIC (PM0214) ------------- */
#define SYST_CSR       (*(volatile uint32_t *)0xE000E010)
#define SYST_RVR       (*(volatile uint32_t *)0xE000E014)
#define SYST_CVR       (*(volatile uint32_t *)0xE000E018)
#define NVIC_ISER0     (*(volatile uint32_t *)0xE000E100)

/* ---------------- linker symbols (addresses only!) ---------------- */
extern uint32_t _estack, _sidata, _sdata, _edata, _sbss, _ebss;

/* ---------------- shared state ---------------- */
volatile uint32_t ticks;            /* .bss  — modified by SysTick ISR  */
volatile uint32_t presses;          /* .bss  — button press counter     */
int blink_delay = 500;              /* .data — modified by the ISR      */

/* =================== UART (week 4) =================== */

static void uart_init(void)
{
    RCC_AHB1ENR |= (1 << 0);        /* GPIOA clock            */
    RCC_APB2ENR |= (1 << 4);        /* USART1 clock           */
    (void)RCC_APB2ENR;              /* readback: let it settle */

    /* PA9/PA10 -> alternate function mode (10) */
    GPIOA_MODER &= ~((3u << (9 * 2)) | (3u << (10 * 2)));
    GPIOA_MODER |=  ((2u << (9 * 2)) | (2u << (10 * 2)));

    /* AF7 = USART1 (datasheet AF mapping table). Pin 9 -> AFRH[7:4], pin 10 -> AFRH[11:8] */
    GPIOA_AFRH &= ~((0xFu << 4) | (0xFu << 8));
    GPIOA_AFRH |=  ((7u << 4) | (7u << 8));

    /* 16 MHz / (16 * 115200) = 8.68 -> mantissa 8, frac round(.6805*16)=11 -> 0x8B */
    USART1_BRR = 0x8B;
    USART1_CR1 = (1 << 13) | (1 << 3) | (1 << 2);   /* UE | TE | RE */
}

static void uart_putc(char c)
{
    while (!(USART1_SR & (1 << 7))) ;               /* TXE */
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

static void uart_puthex(uint32_t v)
{
    for (int shift = 28; shift >= 0; shift -= 4) {
        uint32_t nib = (v >> shift) & 0xF;
        uart_putc(nib < 10 ? '0' + nib : 'A' + nib - 10);
    }
}

static void uart_putdec(uint32_t v)
{
    char buf[10];
    int i = 0;
    do {
        buf[i++] = '0' + (v % 10);
        v /= 10;
    } while (v);
    while (i--)
        uart_putc(buf[i]);
}

/* =================== time (week 5) =================== */

static void systick_init(void)
{
    SYST_RVR = 16000000 / 1000 - 1;  /* 1 ms @ 16 MHz: 15999 (count N transitions -> N-1) */
    SYST_CVR = 0;                    /* writing anything clears the counter */
    SYST_CSR = (1 << 2) | (1 << 1) | (1 << 0);   /* CLKSOURCE=CPU | TICKINT | ENABLE */
}

static uint32_t millis(void)
{
    return ticks;
}

static void delay_ms(uint32_t ms)
{
    uint32_t start = millis();
    while (millis() - start < ms) ;  /* unsigned math: correct across tick wraparound */
}

/* =================== button (week 5) =================== */

static void button_init(void)
{
    RCC_AHB1ENR |= (1 << 0);         /* GPIOA clock (PA0 input is reset default mode) */
    RCC_APB2ENR |= (1 << 14);        /* SYSCFG clock */
    (void)RCC_APB2ENR;

    SYSCFG_EXTICR1 &= ~0xFu;         /* EXTI0 <- port A (0000; reset default, set explicitly) */
    EXTI_RTSR |= (1 << 0);           /* rising edge = button press on this board */
    EXTI_IMR  |= (1 << 0);           /* unmask line 0 in EXTI...   */
    NVIC_ISER0 = (1 << 6);           /* ...and enable IRQ 6 in NVIC (both gates!) */
}

/* =================== handlers =================== */

void systick_handler(void)
{
    ticks++;
}

void exti0_handler(void)
{
    EXTI_PR = (1 << 0);              /* write-1-to-clear, plain =, never |= */

    static uint32_t last;
    if (millis() - last < 50)        /* debounce window */
        return;
    last = millis();

    presses++;
    blink_delay = (blink_delay == 500) ? 100 : 500;
}

void default_handler(void)
{
    for (;;) ;                       /* GDB parked here = unhandled exception */
}

/* =================== main =================== */

int main(void)
{
    uart_init();
    systick_init();
    button_init();

    RCC_AHB1ENR |= (1 << 6);         /* GPIOG clock */
    (void)RCC_AHB1ENR;
    GPIOG_MODER &= ~(3u << (13 * 2));
    GPIOG_MODER |=  (1u << (13 * 2));

    uart_puts("\n=== week05 bare-metal boot ===\n");
    uart_puts("SYSCLK: ");
    uart_putdec(16000000);
    uart_puts(" Hz (HSI)\n");

    uint32_t last_print = 0;

    while (1) {
        GPIOG_ODR ^= (1 << 13);
        delay_ms((uint32_t)blink_delay);

        if (millis() - last_print >= 1000) {
            last_print += 1000;
            uart_puts("uptime: ");
            uart_putdec(millis() / 1000);
            uart_puts(" s | presses: ");
            uart_putdec(presses);
            uart_puts(" | blink_delay @ ");
            uart_puthex((uint32_t)&blink_delay);
            uart_puts(" = ");
            uart_putdec((uint32_t)blink_delay);
            uart_puts("\n");
        }
    }
}

/* =================== startup (week 3) =================== */
void reset_handler(void)
{
    uint32_t *src = &_sidata;
    for (uint32_t *dst = &_sdata; dst < &_edata; )
        *dst++ = *src++;
    for (uint32_t *dst = &_sbss; dst < &_ebss; )
        *dst++ = 0;
    main();
    while (1) ;
}

/* =================== vector table (PM0214 + RM0090 table 62) =================== */

__attribute__((section(".isr_vector")))
const void *vector_table[] = {
    &_estack,           /*  0: initial SP                */
    reset_handler,      /*  1: Reset                     */
    default_handler,    /*  2: NMI                       */
    default_handler,    /*  3: HardFault                 */
    default_handler,    /*  4: MemManage                 */
    default_handler,    /*  5: BusFault                  */
    default_handler,    /*  6: UsageFault                */
    0, 0, 0, 0,         /*  7-10: reserved               */
    default_handler,    /* 11: SVCall                    */
    default_handler,    /* 12: Debug monitor             */
    0,                  /* 13: reserved                  */
    default_handler,    /* 14: PendSV  (week 7's star)   */
    systick_handler,    /* 15: SysTick                   */
    0, 0, 0, 0, 0, 0,   /* 16-21: IRQ 0..5               */
    exti0_handler,      /* 22: IRQ 6 = EXTI0             */
};