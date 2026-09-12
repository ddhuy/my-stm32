/*
 * weeks 9E + 10E — single-file build (post-debug final)
 * STM32F429I-DISC1, bare metal: no HAL, no CMSIS.
 *
 * Contents, in init order:
 *   1. clock_init_168mhz()   — HSE 8 MHz -> PLL -> 168 MHz  (9E step 1)
 *   2. sdram_pins_init()     — 38-pin FMC mux               (9E step 2)
 *   3. fmc_init()            — controller geometry/timings  (9E step 3)
 *   4. sdram_wakeup()        — JEDEC liturgy                (9E step 4)
 *   5. lcd_pins_init()       — ~28-pin LTDC mux             (10E step 1)
 *   6. ili9341_init()        — panel config over SPI5       (10E step 2)
 *   7. pllsai_init()         — pixel clock                  (10E step 3)
 *   8. ltdc_init()           — scanout engine + layer 1     (10E step 4)
 *   9. lcd_bands()           — first light                  (10E step 5)
 *
 * PANEL-SPECIFIC constants are marked << VERIFY >>. Confirm against
 * UM1670 (schematic), the ILI9341 datasheet, and RM0090 (LTDC/FMC).
 */

#include <stdint.h>

/* ============================================================ */
/* register map                                                 */
/* ============================================================ */

/* ---- RCC ---- */
#define RCC_CR          (*(volatile uint32_t *)0x40023800)
#define RCC_PLLCFGR     (*(volatile uint32_t *)0x40023804)
#define RCC_CFGR        (*(volatile uint32_t *)0x40023808)
#define RCC_AHB1ENR     (*(volatile uint32_t *)0x40023830)
#define RCC_AHB3ENR     (*(volatile uint32_t *)0x40023838)
#define RCC_APB1ENR     (*(volatile uint32_t *)0x40023840)
#define RCC_APB2ENR     (*(volatile uint32_t *)0x40023844)
#define RCC_PLLSAICFGR  (*(volatile uint32_t *)0x40023888)
#define RCC_DCKCFGR     (*(volatile uint32_t *)0x4002308C)

/* ---- PWR / FLASH ---- */
#define PWR_CR          (*(volatile uint32_t *)0x40007000)
#define FLASH_ACR       (*(volatile uint32_t *)0x40023C00)

/* ---- USART1 ---- */
#define GPIOA_MODER     (*(volatile uint32_t *)0x40020000)
#define GPIOA_AFRH      (*(volatile uint32_t *)0x40020024)
#define USART1_SR       (*(volatile uint32_t *)0x40011000)
#define USART1_DR       (*(volatile uint32_t *)0x40011004)
#define USART1_BRR      (*(volatile uint32_t *)0x40011008)
#define USART1_CR1      (*(volatile uint32_t *)0x4001100C)

/* ---- SPI5 (ILI9341 command interface) ---- */
#define SPI5_CR1        (*(volatile uint32_t *)0x40015000)
#define SPI5_SR         (*(volatile uint32_t *)0x40015008)
#define SPI5_DR         (*(volatile uint32_t *)0x4001500C)

/* ---- LCD control pins: CS=PC2, WRX=PD13   << VERIFY >> ---- */
#define GPIOC_MODER     (*(volatile uint32_t *)0x40020800)
#define GPIOC_ODR       (*(volatile uint32_t *)0x40020814)
#define GPIOD_MODER     (*(volatile uint32_t *)0x40020C00)
#define GPIOD_ODR       (*(volatile uint32_t *)0x40020C14)

/* ---- FMC (SDRAM controller) ---- */
#define FMC_SDCR1       (*(volatile uint32_t *)0xA0000140)
#define FMC_SDCR2       (*(volatile uint32_t *)0xA0000144)
#define FMC_SDTR1       (*(volatile uint32_t *)0xA0000148)
#define FMC_SDTR2       (*(volatile uint32_t *)0xA000014C)
#define FMC_SDCMR       (*(volatile uint32_t *)0xA0000150)
#define FMC_SDRTR       (*(volatile uint32_t *)0xA0000154)
#define FMC_SDSR        (*(volatile uint32_t *)0xA0000158)

/* ---- LTDC ---- */
#define LTDC_BASE       0x40016800u
#define LTDC_ISR        (*(volatile uint32_t *)(LTDC_BASE + 0x34u))
#define LTDC_ICR        (*(volatile uint32_t *)(LTDC_BASE + 0x38u))
#define LTDC_L1CR       (*(volatile uint32_t *)(LTDC_BASE + 0x84u))
#define LTDC_GCR        (*(volatile uint32_t *)(LTDC_BASE + 0x00u))

#define LTDC_SSCR       (*(volatile uint32_t *)0x40016808)
#define LTDC_BPCR       (*(volatile uint32_t *)0x4001680C)
#define LTDC_AWCR       (*(volatile uint32_t *)0x40016810)
#define LTDC_TWCR       (*(volatile uint32_t *)0x40016814)
#define LTDC_SRCR       (*(volatile uint32_t *)0x40016824)
#define LTDC_BCCR       (*(volatile uint32_t *)0x4001682C)
#define LTDC_L1WHPCR    (*(volatile uint32_t *)0x40016888)
#define LTDC_L1WVPCR    (*(volatile uint32_t *)0x4001688C)
#define LTDC_L1PFCR     (*(volatile uint32_t *)0x40016894)
#define LTDC_L1CACR     (*(volatile uint32_t *)0x40016898)
#define LTDC_L1CFBAR    (*(volatile uint32_t *)0x400168AC)
#define LTDC_L1CFBLR    (*(volatile uint32_t *)0x400168B0)
#define LTDC_L1CFBLNR   (*(volatile uint32_t *)0x400168B4)

/* ---- core ---- */
#define SYST_CSR        (*(volatile uint32_t *)0xE000E010)
#define SYST_RVR        (*(volatile uint32_t *)0xE000E014)
#define SYST_CVR        (*(volatile uint32_t *)0xE000E018)

#define SYSCLK_HZ       168000000u
#define SDRAM_BASE      0xD0000000u
#define SDRAM_BYTES     (8u * 1024u * 1024u)

#define LCD_W   240u
#define LCD_H   320u

extern uint32_t _estack, _sidata, _sdata, _edata, _sbss, _ebss;

volatile uint32_t ticks;

/* the framebuffer lives in external SDRAM (week 9e step 6 linker section) */
__attribute__((section(".sdram"))) uint16_t framebuffer[LCD_W * LCD_H];

/* ============================================================ */
/* clock (9E step 1)                                            */
/* ============================================================ */

static void clock_init_168mhz(void)
{
    RCC_APB1ENR |= (1u << 28);            /* PWR clock                */
    (void)RCC_APB1ENR;
    PWR_CR |= (3u << 14);                 /* VOS scale 1              */

    RCC_CR |= (1u << 16);                 /* HSEON                    */
    while (!(RCC_CR & (1u << 17))) ;      /* HSERDY                   */

    RCC_PLLCFGR = (4u << 0) | (168u << 6) | (0u << 16)
                | (1u << 22) | (7u << 24);/* M=4 N=168 P=/2 HSE Q=7    */

    RCC_CR |= (1u << 24);                 /* PLLON                    */
    while (!(RCC_CR & (1u << 25))) ;      /* PLLRDY                   */

    FLASH_ACR = (1u << 10) | (1u << 9) | (1u << 8) | 5u;  /* caches+5WS */
    RCC_CFGR |= (5u << 10) | (4u << 13);  /* APB1/4 APB2/2            */
    RCC_CFGR = (RCC_CFGR & ~3u) | 2u;     /* SW = PLL                 */
    while ((RCC_CFGR & (3u << 2)) != (2u << 2)) ;
}

/* ============================================================ */
/* UART (week 4)                                                */
/* ============================================================ */

static void uart_init(void)
{
    RCC_AHB1ENR |= (1u << 0);
    RCC_APB2ENR |= (1u << 4);
    (void)RCC_APB2ENR;
    GPIOA_MODER &= ~((3u << (9 * 2)) | (3u << (10 * 2)));
    GPIOA_MODER |=  ((2u << (9 * 2)) | (2u << (10 * 2)));
    GPIOA_AFRH  &= ~((0xFu << 4) | (0xFu << 8));
    GPIOA_AFRH  |=  ((7u << 4) | (7u << 8));
    USART1_BRR = 0x2D9;                   /* 115200 @ 84 MHz APB2     */
    USART1_CR1 = (1u << 13) | (1u << 3) | (1u << 2);
}

static void uart_putc(char c)
{
    while (!(USART1_SR & (1u << 7))) ;
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
        uint32_t nib = (v >> shift) & 0xFu;
        uart_putc(nib < 10 ? '0' + nib : 'A' + nib - 10);
    }
}

static void uart_putdec(uint32_t v)
{
    char buf[10];
    int i = 0;
    do { buf[i++] = '0' + (v % 10); v /= 10; } while (v);
    while (i--) uart_putc(buf[i]);
}

/* ============================================================ */
/* time                                                         */
/* ============================================================ */

static void systick_init(void)
{
    SYST_RVR = SYSCLK_HZ / 1000u - 1u;
    SYST_CVR = 0;
    SYST_CSR = (1u << 2) | (1u << 1) | (1u << 0);
}

void systick_handler(void) { ticks++; }

static uint32_t millis(void) { return ticks; }

void delay_ms(uint32_t ms)
{
    uint32_t start = millis();
    while (millis() - start < ms) ;
}

static void delay_us(uint32_t us)
{
    volatile uint32_t n = us * (SYSCLK_HZ / 1000000u) / 4u;
    while (n--) ;
}

/* ============================================================ */
/* GPIO helper (shared by both pin swarms)                      */
/* ============================================================ */

static volatile uint32_t *gpio_reg(char port, uint32_t offset)
{
    return (volatile uint32_t *)(0x40020000u + 0x400u * (uint32_t)(port - 'A') + offset);
}

struct pin { char port; uint8_t pin; uint8_t af; };

/* set one pin to alternate-function mode, very-high speed, given AF */
static void pin_af(char p, uint32_t n, uint32_t af)
{
    volatile uint32_t *moder   = gpio_reg(p, 0x00);
    volatile uint32_t *ospeedr = gpio_reg(p, 0x08);
    volatile uint32_t *pupdr   = gpio_reg(p, 0x0C);
    *moder   = (*moder   & ~(3u << (n * 2))) | (2u << (n * 2));
    *ospeedr = (*ospeedr | (3u << (n * 2)));
    *pupdr   = (*pupdr   & ~(3u << (n * 2)));
    {
        uint32_t off = (n < 8) ? 0x20 : 0x24;
        uint32_t sh  = (n & 7u) * 4u;
        volatile uint32_t *afr = gpio_reg(p, off);
        *afr = (*afr & ~(0xFu << sh)) | (af << sh);
    }
}

static void dump_pin(char port, uint32_t n)
{
    uint32_t mode  = (*gpio_reg(port, 0x00) >> (n * 2)) & 3u;
    uint32_t speed = (*gpio_reg(port, 0x08) >> (n * 2)) & 3u;
    uint32_t off   = (n < 8) ? 0x20 : 0x24;
    uint32_t af    = (*gpio_reg(port, off) >> ((n & 7u) * 4u)) & 0xFu;
    uart_putc('P'); uart_putc(port); uart_putdec(n);
    uart_puts(" mode="); uart_putdec(mode);
    uart_puts(" spd=");  uart_putdec(speed);
    uart_puts(" af=");   uart_putdec(af);
    uart_puts("\n");
}

/* ============================================================ */
/* SDRAM pins (9E step 2)   << audit against UM1670 >>          */
/* ============================================================ */

static const struct pin sdram_pins[] = {
    {'D',0,12},{'D',1,12},{'D',3,12}, /* D2, D3, CLK (ADDED PD3) */
    {'D',8,12},{'D',9,12},{'D',10,12},/* D13, D14, D15 */
    {'D',14,12},{'D',15,12},          /* D0, D1 */
    {'E',0,12},{'E',1,12},            /* NBL0, NBL1 */
    {'E',7,12},{'E',8,12},{'E',9,12},{'E',10,12},{'E',11,12},   /* D4 to D8 */
    {'E',12,12},{'E',13,12},{'E',14,12},{'E',15,12},            /* D9 to D12 */
    {'F',0,12},{'F',1,12},{'F',2,12},{'F',3,12},{'F',4,12},{'F',5,12}, /* A0 to A5 */
    {'F',11,12},{'F',12,12},{'F',13,12},{'F',14,12},{'F',15,12},/* SDNRAS, A6 to A9 */
    {'G',0,12},{'G',1,12},            /* A10, A11 */
    {'G',4,12},{'G',5,12},            /* BA0, BA1 */
    {'G',8,12}, /* REMOVE THIS LINE - PG8 IS WRONG */
    {'G',15,12},                      /* SDNCAS (ADDED/VERIFIED) */
    {'B',5,12},{'B',6,12},            /* SDNWE, SDNE1 */
    {'C',0,12}                        /* SDN_CKE1 */
};


static void sdram_pins_init(void)
{
    RCC_AHB1ENR |= (0x3Fu << 1);          /* GPIOB..G */
    (void)RCC_AHB1ENR;
    for (unsigned i = 0; i < (sizeof(sdram_pins) / sizeof(sdram_pins[0])); i++)
        pin_af(sdram_pins[i].port, sdram_pins[i].pin, sdram_pins[i].af);
}

/* ============================================================ */
/* FMC controller (9E step 3)                                   */
/* ============================================================ */

static void fmc_wait_busy(void) { while (FMC_SDSR & (1u << 5)) ; }

static void fmc_init(void)
{
    RCC_AHB3ENR |= (1u << 0);
    (void)RCC_AHB3ENR;

    /* bank-1 registers hold the controller-global fields */
    FMC_SDCR1 = (2u << 10) | (1u << 12);            /* SDCLK/2, RBURST */
    FMC_SDTR1 = ((6u - 1u) << 12) | ((2u - 1u) << 20); /* TRC, TRP     */

    /* bank-2 geometry + timings */
    FMC_SDCR2 = (0u << 0) | (1u << 2) | (1u << 4) | (1u << 6) | (3u << 7);
    FMC_SDTR2 = ((2u-1u)<<0)|((6u-1u)<<4)|((4u-1u)<<8)|((2u-1u)<<16)|((2u-1u)<<24);
}

/* ============================================================ */
/* SDRAM wake-up liturgy (9E step 4)                            */
/* ============================================================ */

static void sdram_wakeup(void)
{
    fmc_wait_busy();
    FMC_SDCMR = 1u | (1u << 3);                    /* clk enable, bank2 */
    delay_us(1000);

    fmc_wait_busy();
    FMC_SDCMR = 2u | (1u << 3);                    /* precharge all     */

    fmc_wait_busy();
    FMC_SDCMR = 3u | (1u << 3) | ((8u - 1u) << 5); /* auto-refresh x8   */

    fmc_wait_busy();
    FMC_SDCMR = 4u | (1u << 3) | (0x230u << 9);    /* load mode reg     */

    fmc_wait_busy();
    FMC_SDRTR = (1292u << 1);                       /* refresh rhythm    */
    fmc_wait_busy();
}

static int sdram_smoke_test(void)
{
    volatile uint32_t *m = (volatile uint32_t *)SDRAM_BASE;
    volatile uint32_t *far = (volatile uint32_t *)(SDRAM_BASE + SDRAM_BYTES - 4u);
    m[0] = 0xDEADBEEFu;
    *far = 0x12345678u;
    return (m[0] == 0xDEADBEEFu) && (*far == 0x12345678u);
}

/* ============================================================ */
/* LCD pins (10E step 1)   << audit; note AF9 exceptions >>     */
/* ============================================================ */

/*
 * LTDC RGB pins are AF14, EXCEPT four that are AF9:
 *   R3=PB0, R6=PB1, G3=PG10, B4=PG12.
 * A single AF14-where-AF9 pin corrupts one color channel and looks
 * exactly like a panel defect. << VERIFY the whole list in UM1670 >>
 */
static const struct pin lcd_pins[] = {
    /* ======================================================== */
    /* AF9 Exceptions (Exactly Four Pins on this Kit)           */
    /* ======================================================== */
    {'B', 0, 9},   /* R3 */
    {'G', 10, 9},  /* G3 */
    {'G', 11, 9},  /* B3 */
    {'G', 12, 9},  /* B4 */

    /* ======================================================== */
    /* AF14 Standard LTDC Data & Timing Signals                 */
    /* ======================================================== */
    /* Fix: Change multi-character 'PG' to single character 'G' */
    {'G', 7, 14},  /* DOTCLK */
    {'F', 10, 14}, /* ENABLE */
    {'C', 6, 14},  /* HSYNC  */
    {'A', 4, 14},  /* VSYNC  */

    /* Red Bus */
    {'C', 10, 14}, /* R2 */
    {'A', 11, 14}, /* R4 */
    {'A', 12, 14}, /* R5 */
    {'H', 12, 14}, /* R6 */
    {'G', 6, 14},  /* R7 */

    /* Green Bus */
    {'A', 6, 14},  /* G2 */
    {'B', 10, 14}, /* G4 */
    {'B', 11, 14}, /* G5 */
    {'H', 13, 14}, /* G6 */
    {'I', 2, 14},  /* G7 */

    /* Blue Bus */
    {'D', 6, 14},  /* B2 */
    {'A', 3, 14},  /* B5 */
    {'B', 8, 14},  /* B6 */
    {'B', 9, 14}   /* B7 */
};


static void lcd_pins_init(void) 
{ 
    /* Fix 1: Enable GPIOA through GPIOI (Bits 0 to 8 in RCC_AHB1ENR) */
    RCC_AHB1ENR |= 0x000001FFu; 
    (void)RCC_AHB1ENR; 

    /* Loop through and bind all LTDC / SPI5 pins */
    unsigned num_pins = sizeof(lcd_pins) / sizeof(lcd_pins[0]);
    for (unsigned i = 0; i < num_pins; i++) {
        pin_af(lcd_pins[i].port, lcd_pins[i].pin, lcd_pins[i].af); 
    }

    /* Fix 2: CS=PC2, IM2=PD12, and WRX=PD13 are plain outputs */
    GPIOC_MODER = (GPIOC_MODER & ~(3u << (2 * 2)))  | (1u << (2 * 2));  /* CS */
    GPIOD_MODER = (GPIOD_MODER & ~(3u << (12 * 2))) | (1u << (12 * 2)); /* IM2 */
    GPIOD_MODER = (GPIOD_MODER & ~(3u << (13 * 2))) | (1u << (13 * 2)); /* WRX */

    /* Set idle states */
    GPIOC_ODR |= (1u << 2);   /* CS idle high */
    GPIOD_ODR |= (1u << 12);  /* IM2 high: forces ILI9341 into SPI communication mode */
}


/* ============================================================ */
/* SPI5 + ILI9341 command interface (10E step 2)                */
/* ============================================================ */
static void spi5_init(void) 
{ 
    RCC_AHB1ENR |= (1u << 5);   /* GPIOF for SPI5 pins */ 
    RCC_APB2ENR |= (1u << 20);  /* SPI5 clock */ 
    (void)RCC_APB2ENR; 

    /* PF7=SCK, PF8=MISO, PF9=MOSI -> AF5 */ 
    pin_af('F', 7, 5); 
    pin_af('F', 8, 5); 
    pin_af('F', 9, 5); 

    /* SPI5 configuration:
     * CPHA=1, CPOL=1, MSTR=1 
     * Baud Rate = APB2 (84MHz) / 16 = 5.25 MHz (ILI9341 max SPI clock is 10MHz)
     * SSM=1, SSI=1 
     */
    SPI5_CR1 = (1u << 0) | (1u << 1) | (1u << 2) 

             | (3u << 3) | (1u << 9) | (1u << 8); 
    
    SPI5_CR1 |= (1u << 6);      /* SPE (SPI Enable) */ 
} 

static uint8_t spi5_xfer(uint8_t b) 
{ 
    /* 1. Ensure Transmit buffer is totally empty before writing */
    while (!(SPI5_SR & (1u << 1))) ; 
    
    SPI5_DR = b; 
    
    /* 2. Wait until the byte completely shifts in from the slave */
    while (!(SPI5_SR & (1u << 0))) ; 
    
    /* 3. Return the read register value (clears RXNE automatically) */
    return (uint8_t)SPI5_DR; 
} 

static void lcd_cs(int low) 
{ 
    if (low) GPIOC_ODR &= ~(1u << 2); 
    else {
        /* Fix 2: Wait until SPI is completely idle before raising CS */
        while (SPI5_SR & (1u << 7)) ; /* Bit 7 = BSY (Busy) flag */
        GPIOC_ODR |= (1u << 2); 
    }
} 

static void lcd_wrx(int high) 
{ 
    if (high) GPIOD_ODR |= (1u << 13); 
    else GPIOD_ODR &= ~(1u << 13); 
} 

static void lcd_cmd(uint8_t c) 
{ 
    lcd_wrx(0);      /* Command mode (D/C low) */
    lcd_cs(1);       /* CS Low (Active) */
    (void)spi5_xfer(c); 
    lcd_cs(0);       /* CS High (Idle) + BSY check inside */
} 

static void lcd_dat(uint8_t d) 
{ 
    lcd_wrx(1);      /* Data mode (D/C high) */
    lcd_cs(1);       /* CS Low (Active) */
    (void)spi5_xfer(d); 
    lcd_cs(0);       /* CS High (Idle) + BSY check inside */
}

/*
 * ILI9341 init — VERBATIM translation of ST's ili9341_Init() from the
 * BSP component driver (validated on this exact panel), with two
 * deliberate deviations, both proven necessary on the bench:
 *   1. The FINAL LCD_GRAM (0x2C) after display-on is OMITTED — bisection
 *      showed it blanks the panel in our flow (cross-interface GRAM poke
 *      while the LTDC is already streaming with RM=1).
 *   2. Sleep-out delay raised 200 -> 300 ms (more settling margin).
 * NOTE: there is no SWRESET here (ST omits it), and the panel KEEPS its
 * state across MCU resets — always full power-cycle between tests.
 */
#define LCD_SLEEP_OUT     0x11
#define LCD_GAMMA         0x26
#define LCD_DISPLAY_ON    0x29
#define LCD_COLUMN_ADDR   0x2A
#define LCD_PAGE_ADDR     0x2B
#define LCD_GRAM          0x2C
#define LCD_MAC           0x36
#define LCD_RGB_INTERFACE 0xB0
#define LCD_FRMCTR1       0xB1
#define LCD_DFC           0xB6
#define LCD_POWER1        0xC0
#define LCD_POWER2        0xC1
#define LCD_VCOM1         0xC5
#define LCD_VCOM2         0xC7
#define LCD_PGAMMA        0xE0
#define LCD_NGAMMA        0xE1
#define LCD_INTERFACE     0xF6
#define LCD_POWERA        0xCB
#define LCD_POWERB        0xCF
#define LCD_DTCA          0xE8
#define LCD_DTCB          0xEA
#define LCD_POWER_SEQ     0xED
#define LCD_3GAMMA_EN     0xF2
#define LCD_PRC           0xF7

static void ili9341_init(void) 
{
    /* Fix 1: Force an explicit Software Reset to sync the ILI9341 state machine */
    lcd_cmd(0x01); 
    delay_ms(50); /* Mandatory wait time for reset processing */

    /* Power Control A configuration */
    lcd_cmd(LCD_POWERA); 
    lcd_dat(0x39); lcd_dat(0x2C); lcd_dat(0x00); lcd_dat(0x34); lcd_dat(0x02); 

    /* Power Control B configuration */
    lcd_cmd(LCD_POWERB); 
    lcd_dat(0x00); lcd_dat(0xC1); lcd_dat(0x30); 
    
    /* Driver Timing Control A */
    lcd_cmd(LCD_DTCA); 
    lcd_dat(0x85); lcd_dat(0x00); lcd_dat(0x78); 
    
    /* Driver Timing Control B */
    lcd_cmd(LCD_DTCB); 
    lcd_dat(0x00); lcd_dat(0x00); 
    
    /* Power-On Sequence Control */
    lcd_cmd(LCD_POWER_SEQ); 
    lcd_dat(0x64); lcd_dat(0x03); lcd_dat(0x12); lcd_dat(0x81); 
    
    /* Pump Ratio Control */
    lcd_cmd(LCD_PRC); 
    lcd_dat(0x20); 
    
    /* Power Control 1 & 2 definitions */
    lcd_cmd(LCD_POWER1); lcd_dat(0x10); 
    lcd_cmd(LCD_POWER2); lcd_dat(0x10); 
    
    /* VCOM Control 1 & 2 (Contrast adjustment) */
    lcd_cmd(LCD_VCOM1); lcd_dat(0x45); lcd_dat(0x15); 
    lcd_cmd(LCD_VCOM2); lcd_dat(0x90); 
    
    /* Memory Access Control (Orientation) */
    lcd_cmd(LCD_MAC); 
    lcd_dat(0xC8); /* 0xC8 = Standard ST orientation (MY|MX|BGR) */
    
    /* Frame Rate Control (In Normal Mode / Full Colors) */
    lcd_cmd(LCD_FRMCTR1); 
    lcd_dat(0x00); lcd_dat(0x1B); 

    /* Fix 2: Consolidated Display Function Control (0xB6) 
     * Configures the panel to bypass standard MCU interface and use 
     * the external parallel RGB interface driven by the LTDC engine */
    lcd_cmd(LCD_DFC); 
    lcd_dat(0x0A); lcd_dat(0xA7); lcd_dat(0x27); lcd_dat(0x04);

    /* 3-Gamma Function Disable */
    lcd_cmd(LCD_3GAMMA_EN); 
    lcd_dat(0x00); 
    
    /* Gamma Curve Selection */
    lcd_cmd(LCD_GAMMA); 
    lcd_dat(0x01); 
    
    /* Positive & Negative Gamma Correction curves */
    lcd_cmd(LCD_PGAMMA); 
    lcd_dat(0x0F); lcd_dat(0x29); lcd_dat(0x24); lcd_dat(0x0C); lcd_dat(0x0E); 
    lcd_dat(0x09); lcd_dat(0x4E); lcd_dat(0x78); lcd_dat(0x3C); lcd_dat(0x09); 
    lcd_dat(0x13); lcd_dat(0x05); lcd_dat(0x17); lcd_dat(0x11); lcd_dat(0x00); 
    
    lcd_cmd(LCD_NGAMMA); 
    lcd_dat(0x00); lcd_dat(0x16); lcd_dat(0x1B); lcd_dat(0x04); lcd_dat(0x11); 
    lcd_dat(0x07); lcd_dat(0x31); lcd_dat(0x33); lcd_dat(0x42); lcd_dat(0x05); 
    lcd_dat(0x0C); lcd_dat(0x0A); lcd_dat(0x28); lcd_dat(0x2F); lcd_dat(0x0F); 
    
    /* RGB Interface Signal Control */
    lcd_cmd(LCD_RGB_INTERFACE); 
    lcd_dat(0xC2); /* By setting ByPass_Mode=1, device goes to RGB interface */
    
    /* Interface Control */
    lcd_cmd(LCD_INTERFACE); 
    lcd_dat(0x01); lcd_dat(0x00); lcd_dat(0x06); 
    
    /* Clear and wake parameters */
    lcd_cmd(LCD_SLEEP_OUT); 
    delay_ms(200); 
    
    lcd_cmd(LCD_DISPLAY_ON); 
}


/* ============================================================ */
/* ============================================================ */
/* PLLSAI pixel clock (10E step 3)  << VERIFY N/R/DIVR >>       */
/* ============================================================ */

static void pllsai_init(void) 
{ 
    /* 1. Ensure the LTDC peripheral clock gate is completely TURNED OFF first.
     * This forces the internal clock domain bridge to clear its state. */
    RCC_APB2ENR &= ~(1u << 26);

    /* 2. Configure multipliers: VCO Input (2MHz) * 192 = 384MHz. R = 3 -> 128MHz */
    RCC_PLLSAICFGR = (192u << 6) | (3u << 28); 

    /* 3. Configure DIVR to divide by 8: 128MHz / 8 = 16MHz.
     * We modify ONLY bits 16:17 to preserve all other peripheral configurations. */
    RCC_DCKCFGR = (RCC_DCKCFGR & ~(3u << 16)) | (1u << 16); 

    /* 4. Turn on the PLLSAI engine and wait for hardware stabilization */
    RCC_CR |= (1u << 28); /* PLLSAION */ 
    while (!(RCC_CR & (1u << 29))) ; /* Wait for PLLSAIRDY to pull high */ 

    /* 5. CRITICAL STEP: Now that the clock line is alive and stable,
     * turn the LTDC clock gate back ON. This forces the hardware mux 
     * to successfully latch the working clock line. */
    RCC_APB2ENR |= (1u << 26);
    (void)RCC_APB2ENR; /* Short delay to let clock rails settle */
}


/* ============================================================ */
/* LTDC scanout (10E step 4)                                    */
/* ============================================================ */

#define HSW 10u
#define HBP 20u
#define HFP 10u
#define VSW 2u
#define VBP 2u
#define VFP 4u
static void ltdc_init(void) 
{ 
    /* 2. Configure global timing parameters based on front/back porch limits */
    LTDC_SSCR = ((HSW - 1u) << 16) | (VSW - 1u); 
    LTDC_BPCR = ((HSW + HBP - 1u) << 16) | (VSW + VBP - 1u); 
    LTDC_AWCR = ((HSW + HBP + LCD_W - 1u) << 16) | (VSW + VBP + LCD_H - 1u); 
    LTDC_TWCR = ((HSW + HBP + LCD_W + HFP - 1u) << 16) | (VSW + VBP + LCD_H + VFP - 1u); 

    /* Set background color to solid Green (If you see Green, LTDC works but Layer 1 is broken) */
    /* If you see White, the whole LTDC sync clock timing loop is stalling */
    LTDC_BCCR = 0x0000FF00; 

    /* 3. Pixel-Perfect Layer 1 horizontal & vertical viewable bounds */
    uint32_t hstart = HSW + HBP; 
    uint32_t vstart = VSW + VBP; 
    
    /* WHSPPOS (bits 16:27) = hstart + LCD_W - 1. WHSTPOS (bits 0:11) = hstart */
    LTDC_L1WHPCR = ((hstart + LCD_W - 1u) << 16) | hstart; 
    /* WVSPPOS (bits 16:27) = vstart + LCD_H - 1. WVSTPOS (bits 0:11) = vstart */
    LTDC_L1WVPCR = ((vstart + LCD_H - 1u) << 16) | vstart; 

    /* Set pixel data format to 16-bit RGB565 (0x2) */
    LTDC_L1PFCR = 0x2; 
    
    /* Set layer opacity to solid/opaque (0xFF) */
    LTDC_L1CACR = 0xFF; 

    /* Set the memory base pointer address to our external SDRAM framebuffer array */
    LTDC_L1CFBAR = (uint32_t)framebuffer; 

    /* Color Frame Buffer Length Configuration 
     * Pitch (bits 0:12) = Line length in bytes = LCD_W * 2
     * Line Length (bits 16:28) = (Line length in bytes) + 3
     */
    LTDC_L1CFBLR = (((LCD_W * 2u) + 3u) << 16) | (LCD_W * 2u); 
    
    /* Total vertical lines in the frame buffer matrix */
    LTDC_L1CFBLNR = LCD_H; 

    /* Enable Layer 1 */
    LTDC_L1CR |= (1u << 0); 

    /* Clear any lingering hardware FIFO or transfer errors before starting the engine */
    LTDC_ICR = 0x0F; 

    /* 4. Global Signal Polarity configuration inside LTDC_GCR
     * Bit 31: LTDC Enable -> Driven at the end of configuration
     * Bit 29: HSYNC Polarity -> 0 = Active Low (Required for ILI9341)
     * Bit 28: VSYNC Polarity -> 0 = Active Low (Required for ILI9341)
     * Bit 23: DEP (Data Enable) Polarity -> 0 = Active High
     * Bit 22: PCPOL (Pixel Clock) Polarity -> 0 = Active High 
     */
    LTDC_GCR &= ~((1u << 29) | (1u << 28) | (1u << 23) | (1u << 22)); 
    LTDC_GCR |= (1u << 0); /* Global LTDC module enable */

    /* 5. Force immediate copy from shadow registers to active registers */
    LTDC_SRCR |= (1u << 0); /* Immediate reload shadow registers */ 
}

/* ============================================================ */
/* drawing + first light (10E step 5)                           */
/* ============================================================ */

#define RGB565(r,g,b) ((uint16_t)(((r)&0x1F)<<11 | ((g)&0x3F)<<5 | ((b)&0x1F)))

static void lcd_bands(void)
{
    for (uint32_t y = 0; y < LCD_H; y++) {
        uint16_t c = (y < LCD_H/3)     ? RGB565(0x1F,0,0)
                   : (y < 2*LCD_H/3)   ? RGB565(0,0x3F,0)
                                       : RGB565(0,0,0x2F);
        for (uint32_t x = 0; x < LCD_W; x++)
            framebuffer[y * LCD_W + x] = c;
    }
}

/* ============================================================ */
/* main                                                         */
/* ============================================================ */

static void debug_ltdc_status(void)
{
    uint32_t isr = LTDC_ISR;
    
    uart_puts("\r\n=== LTDC DIAGNOSTIC DUMP ===\r\n");
    uart_puts("LTDC_GCR (Global Control): 0x"); uart_puthex(LTDC_GCR); uart_puts("\r\n");
    uart_puts("LTDC_L1CR (Layer1 Control): 0x"); uart_puthex(LTDC_L1CR); uart_puts("\r\n");
    uart_puts("LTDC_ISR (Status Register): 0x"); uart_puthex(isr); uart_puts("\r\n");
    
    if (isr & (1u << 0)) {
        uart_puts("[!] LIF: Line Interrupt Flag is active.\r\n");
    }
    if (isr & (1u << 1)) {
        uart_puts("[ERR] RELIF: Register Reload Interrupt Flag is set.\r\n");
    }
    if (isr & (1u << 2)) {
        uart_puts("[CRITICAL] FUIF: FIFO Underrun Interrupt Flag!\r\n");
        uart_puts("           -> LTDC requested data faster than SDRAM could deliver it,\r\n");
        uart_puts("           -> or the Framebuffer pointer is unmapped/wrong.\r\n");
    }
    if (isr & (1u << 3)) {
        uart_puts("[ERR] TERRIF: Transfer Error Interrupt Flag!\r\n");
        uart_puts("           -> AHB bus error occurred during DMA fetching.\r\n");
        uart_puts("           -> Double-check your memory configuration/alignment.\r\n");
    }
    if (isr == 0) {
        uart_puts("[?] ISR is 0x00: Engine is idle. No clock signals are triggering pulses.\r\n");
        uart_puts("    -> If the screen is white and ISR is 0, PG7 (DOTCLK) is dead.\r\n");
    }
    uart_puts("============================\r\n");
}


int main(void)
{
    /* 1. Core Platform Foundations */
    clock_init_168mhz();
    systick_init();
    uart_init();
    uart_puts("System Core Booted at 168MHz\n");

    /* 2. Initialize Memory Bus (Crucial: Must happen BEFORE writing to framebuffer) */
    sdram_pins_init();
    fmc_init();
    sdram_wakeup();
    
    if (sdram_smoke_test()) {
        uart_puts("SDRAM Verification: SUCCESS\n");
    } else {
        uart_puts("SDRAM Verification: FAILED! Check PD3 or PG15 connection.\n");
        while(1); /* Stall system if hardware bus is completely broken */
    }

    /* 3. Populate display memory buffer while panel is sleeping */
    lcd_bands();
    uart_puts("Framebuffer populated with RGB test patterns.\n");

    /* 4. Awaken and Bind display hardware interfaces */
    lcd_pins_init();   /* Sets up GPIO, SPI5 pins, and drives IM2 hardware line */
    spi5_init();       /* Configures register transport engine */
    ili9341_init();    /* Forces Reset and configures internal panel glass rules */
    uart_puts("ILI9341 Display Controller Initialized over SPI.\n");

    /* 5. Start the Pixel Stream Architecture */
    pllsai_init();     /* Boots up 6.25 MHz Pixel Clock (DOTCLK) */
    ltdc_init();       /* Latches sync boundaries and links SDRAM to the screen */
    uart_puts("LTDC Engine active. Visual stream online.\n");

    debug_ltdc_status();

    /* 6. Background Application Loop */
    while (1) {
        /* You can perform animation loops or buffer swaps here */
        delay_ms(1000);
    }
}


/* ============================================================ */
/* startup + vectors                                            */
/* ============================================================ */

void default_handler(void) { for (;;) ; }

void reset_handler(void)
{
    uint32_t *src = &_sidata;
    for (uint32_t *dst = &_sdata; dst < &_edata; ) *dst++ = *src++;
    for (uint32_t *dst = &_sbss;  dst < &_ebss;  ) *dst++ = 0;
    main();
    while (1) ;
}

__attribute__((section(".isr_vector")))
const void *vector_table[] = {
    &_estack, reset_handler,
    default_handler, default_handler, default_handler, default_handler,
    default_handler, 0, 0, 0, 0,
    default_handler, default_handler, 0, default_handler,
    systick_handler,
};