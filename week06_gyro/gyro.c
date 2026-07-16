#include <stdint.h>

/* ---------------- RCC (RM0090 ch. 6) ---------------- */
#define RCC_AHB1ENR    (*(volatile uint32_t *)0x40023830)
#define RCC_APB2ENR    (*(volatile uint32_t *)0x40023844)

/* ---------------- GPIO (RM0090 ch. 8) ---------------- */
#define GPIOA_MODER    (*(volatile uint32_t *)0x40020000)
#define GPIOA_AFRH     (*(volatile uint32_t *)0x40020024)  /* AFR[1]: pins 8..15 */

#define GPIOC_MODER    (*(volatile uint32_t *)0x40020800)
#define GPIOC_ODR      (*(volatile uint32_t *)0x40020814)
#define GPIOC_AFRL     (*(volatile uint32_t *)0x40020820)
#define GPIOC_AFRH     (*(volatile uint32_t *)0x40020824)

#define GPIOF_MODER    (*(volatile uint32_t *)0x40021400)
#define GPIOF_ODR      (*(volatile uint32_t *)0x40021414)
#define GPIOF_AFRL     (*(volatile uint32_t *)0x40021420)
#define GPIOF_AFRH     (*(volatile uint32_t *)0x40021424)

#define GPIOG_MODER    (*(volatile uint32_t *)0x40021800)
#define GPIOG_ODR      (*(volatile uint32_t *)0x40021814)

/* ---------------- USART1 (RM0090 ch. 30) ---------------- */
#define USART1_SR      (*(volatile uint32_t *)0x40011000)
#define USART1_DR      (*(volatile uint32_t *)0x40011004)
#define USART1_BRR     (*(volatile uint32_t *)0x40011008)
#define USART1_CR1     (*(volatile uint32_t *)0x4001100C)

/* ------------- SPI5 (APB2, RM0090 ch. 28) ------------- */
#define SPI5_CR1       (*(volatile uint32_t *) 0x40015000)
#define SPI5_SR        (*(volatile uint32_t *) 0x40015008)
#define SPI5_DR        (*(volatile uint32_t *) 0x4001500C)

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

/* ---------------- gyro registers (L3GD20 / I3G4250D datasheets) ---------------- */
#define GYRO_WHO_AM_I  0x0F
#define GYRO_CTRL_REG1 0x20
#define GYRO_OUT_X_L   0x28
#define GYRO_READ      0x80    /* address bit 7: read              */
#define GYRO_AUTOINC   0x40    /* address bit 6: auto-increment    */

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

static void uart_puthex8(uint8_t v)
{
    for (int shift = 8; shift >= 0; shift -= 4) {
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

static void uart_putsdec(int32_t v)
{
    if (v < 0) {
        uart_putc('-');
        v = -v;
    }
    uart_putdec((uint32_t)v);
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

/* =================== SPI - Gyro (week 6) =================== */

static void cs_low(void)
{
    GPIOC_ODR &= ~(1 << 1);
}

static void cs_high(void)
{
    GPIOC_ODR |= (1 << 1);
}

static void spi5_init(void)
{
    RCC_AHB1ENR |= (1 << 5) | (1 << 2); // GPIOF + GPIOC
    RCC_APB2ENR |= (1 << 20);           // SPI5 clock
    (void)RCC_APB2ENR;

    // PC1: plain output, start HIGH (deselected!) before anything else
    cs_high();
    GPIOC_MODER = (GPIOC_MODER & ~(3 << 2)) | (1 << 2);

    // PF7/PF8/PF9 -> alternate function mode (10)
    GPIOF_MODER &= ~((3 << (7 * 2)) | (3 << (8 * 2)) | (3 << (9 * 2)));
    GPIOF_MODER |=  ((2 << (7 * 2)) | (2 << (8 * 2)) | (2 << (9 * 2)));
 
    // AF5 = SPI5. Pin 7 in AFRL bits 31:28; pins 8,9 in AFRH bits 3:0, 7:4
    GPIOF_AFRL = (GPIOF_AFRL & ~(0xF << 28)) | (5 << 28);
    GPIOF_AFRH = (GPIOF_AFRH & ~((0xF << 0) | (0xF << 4)))
               | ((5 << 0) | (5 << 4));
 
    // Mode 3 (CPOL=1 CPHA=1), master, fPCLK/16 = 1 MHz, software NSS.
    // SSM+SSI: manage NSS in software and hold it high internally,
    // otherwise the peripheral sees NSS low and drops out of master
    // mode with a MODF fault. Configure first, enable SPE last.
    SPI5_CR1 = (1 << 0)                  // CPHA
             | (1 << 1)                  // CPOL
             | (1 << 2)                  // MSTR
             | (3 << 3)                  // BR = 011: /16
             | (1 << 9) | (1 << 8);      // SSM | SSI
    SPI5_CR1 |= (1 << 6);                // SPE: enable
}

static uint8_t spi5_xfer(uint8_t out)
{
    while (!(SPI5_SR & (1 << 1))) ;  // TXE
    SPI5_DR = out;
    while (!(SPI5_SR & (1 << 0))) ;  // RXNE
    return SPI5_DR;
}

static uint8_t gyro_read(uint8_t reg)
{
    cs_low();
    spi5_xfer(0x80 | reg);       // bit7=1: read
    uint8_t v = spi5_xfer(0x00); // dummout out, data in
    cs_high();
    return v;
}

static void gyro_write(uint8_t reg, uint8_t data)
{
    cs_low();
    spi5_xfer(reg);
    spi5_xfer(data);
    cs_high();
}

static void gyro_read_xyz(int16_t out[3])
{
    uint8_t raw[6];

    cs_low();
    spi5_xfer(GYRO_READ | GYRO_AUTOINC | GYRO_OUT_X_L);
    for (uint8_t i = 0; i < 6; ++i)
        raw[i] = spi5_xfer(0x00);
    cs_high();

    // Little endian pairs
    out[0] = (int16_t)((raw[1] << 8) | raw[0]);
    out[1] = (int16_t)((raw[3] << 8) | raw[2]);
    out[2] = (int16_t)((raw[5] << 8) | raw[4]);
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
    spi5_init();
    button_init();

    RCC_AHB1ENR |= (1 << 6);         /* GPIOG clock */
    (void)RCC_AHB1ENR;
    GPIOG_MODER &= ~(3u << (13 * 2));
    GPIOG_MODER |=  (1u << (13 * 2));

    uart_puts("\n=== week06 bare-metal boot ===\n");
    uart_puts("SYSCLK: ");  uart_putdec(16000000);  uart_puts(" Hz (HSI)\n");
    uart_puts("blink_delay @ ");  uart_puthex((uint32_t) &blink_delay);
    uart_puts(" = ");  uart_puthex((uint32_t) blink_delay); uart_puts("\n");

    uint8_t who = gyro_read(GYRO_WHO_AM_I);
    uart_puts("WHO_AM_I: 0x");
    uart_puthex8(who);
    if (who == 0xD4)
        uart_puts(" (L3GD20)\n");
    else if (who == 0xD3)
        uart_puts(" (I3G4250D — later board revision)\n");
    else
        uart_puts(" (unexpected — check CS, AF5, SPI mode)\n");

    // wake the gyro: normal mode, X/Y/Z enabled
    gyro_write(GYRO_CTRL_REG1, 0x0F);
    delay_ms(50);  // let the first samples land

    // calibration: average 100 at-rest sampels to estimate value
    uart_puts("calibrating, keep the board still...\n");
    int32_t bias[3] = { 0, 0, 0};
    int16_t xyz[3];
    for (uint8_t i = 0; i < 100; ++i) {
        gyro_read_xyz(xyz);
        bias[0] += xyz[0];
        bias[1] += xyz[1];
        bias[2] += xyz[2];
        delay_ms(11);  // ~95 Hz ODR: wait one sample
    }

    bias[0] /= 100;
    bias[1] /= 100;
    bias[2] /= 100;

    uint32_t last_print = 0;

    while (1) {
        if (millis() - last_print >= 100) {
            last_print += 100;
    
            GPIOG_ODR ^= (1 << 13);

            gyro_read_xyz(xyz);

            uart_puts("X: ");
            uart_putsdec(xyz[0] - bias[0]);
            uart_puts("  Y: ");
            uart_putsdec(xyz[1] - bias[1]);
            uart_puts("  Z: ");
            uart_putsdec(xyz[2] - bias[2]);
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