#include <stdint.h>

/* ---------------- RCC (RM0090 ch. 6) ---------------- */
#define RCC_CR         (*(volatile uint32_t *)0x40023800)
#define RCC_PLLCFGR    (*(volatile uint32_t *)0x40023804)
#define RCC_CFGR       (*(volatile uint32_t *)0x40023808)
#define RCC_AHB1ENR    (*(volatile uint32_t *)0x40023830)
#define RCC_APB1ENR    (*(volatile uint32_t *)0x40023840)
#define RCC_APB2ENR    (*(volatile uint32_t *)0x40023844)

/* ---------------- PWR + FLASH ---------------- */
#define PWR_CR         (*(volatile uint32_t *)0x40007000)
#define FLASH_ACR      (*(volatile uint32_t *)0x40023C00)

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
#define SCB_ICSR       (*(volatile uint32_t *)0xE000ED04)   /* bit 28: PENDSVSET  */
#define SCB_SHPR3      (*(volatile uint32_t *)0xE000ED20)   /* PendSV pri: 23:16  */       

/* ---------------- gyro registers (L3GD20 / I3G4250D datasheets) ---------------- */
#define GYRO_WHO_AM_I  0x0F
#define GYRO_CTRL_REG1 0x20
#define GYRO_OUT_X_L   0x28
#define GYRO_READ      0x80    /* address bit 7: read              */
#define GYRO_AUTOINC   0x40    /* address bit 6: auto-increment    */

/* ---------------- Global Macros ---------------- */
#define SYSCLK_HZ      168000000u
#define APB2_HZ        84000000u

/* ---------------- linker symbols (addresses only!) ---------------- */
extern uint32_t _estack, _sidata, _sdata, _edata, _sbss, _ebss;

/* ---------------- shared state ---------------- */
volatile uint32_t ticks;            /* .bss  — modified by SysTick ISR  */
volatile uint32_t presses;          /* .bss  — button press counter     */
int blink_delay = 500;              /* .data — modified by the ISR      */

/* =================== the scheduler's entire database =================== */
#define NTASKS      3
#define STACK_SIZE  256         // 1KB per task
#define STACK_PAINT 0xA5A5A5A5  // high-water-mark dye

struct tcb {
    uint32_t *sp;    /* saved PSP while frozen.
                        MUST remain the first (only) member:
                        pendsv.S reaches it as tcbs + 4*index. */
};

struct tcb          tcbs[NTASKS];               // .bss
uint32_t            stacks[NTASKS][STACK_SIZE]; // .bss: 3KB
volatile int        current;                    // index of the current task
static volatile int sched_started;              // gate for PendSV

// referenced from pendsv.S
void pendsv_handler(void);
void start_first_task(int, void (*entry)(void));

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

    /* 84 MHz / (16 * 115200) = 45.573 -> mantissa 45, frac round 9 -> 0x2D9 */
    USART1_BRR = 0x2D9;
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
    for (int shift = 4; shift >= 0; shift -= 4) {
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
    SYST_RVR = SYSCLK_HZ / 1000 - 1;  /* 1 ms @ 168 MHz: 167999 (count N transitions -> N-1) */
    SYST_CVR = 0;                     /* writing anything clears the counter */
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
    if (sched_started)
        SCB_ICSR = (1 << 28);   /* requests: pend PendSV.
                                   The switch itself waits
                                   for the quiet moment */
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

/* =================== layer 7: the forgery =================== */

static void task_init(int i, void (*entry)(void))
{
    // high-water-mark dye
    for (int k = 0; k < STACK_SIZE; ++k)
        stacks[i][k] = STACK_PAINT;

    uint32_t *sp = &stacks[i][STACK_SIZE];

    /* forge the 8-word hardware frame, in the hardware's push order:
       highest address first.                                             */
    *--sp = 0x01000000;                       /* xPSR: Thumb bit, week 2  */
    *--sp = (uint32_t)entry;                  /* pc: birth address        */
    *--sp = 0xDEADBEEF;                       /* lr: return-trap          */
    *--sp = 0;                                /* r12                      */
    *--sp = 0;                                /* r3                       */
    *--sp = 0;                                /* r2                       */
    *--sp = 0;                                /* r1                       */
    *--sp = 0;                                /* r0                       */

    /* forge the software half that pendsv.S will pop: r4-r11 */
    for (int k = 0; k < 8; k++)
        *--sp = 0;

    tcbs[i].sp = sp;                          /* the single thread to pull */
}

/* =================== the tasks =================== */

static void task_green(void)
{
    while (1) {
        GPIOG_ODR ^= (1 << 13);
        delay_ms(blink_delay);
    }
}

static void task_red(void)
{
    while (1) {
        GPIOG_ODR ^= (1 << 14);
        delay_ms(300);
    }
}

static void task_uart(void)
{
    while (1) {
        uart_puts("alive: ");
        uart_putdec(millis() / 1000);
        uart_puts(" s\n");
        delay_ms(1000);
    }
}

static void task_gyro(void)
{
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

/* =================== Clock Init =================== */
static void clock_init_168mhz(void)
{
    /**
     * Core supply first: PWR clock on, voltage scale 1 (VOS=0b11)
     * Raise the supple before raising the demand
     */
    RCC_APB1ENR |= (1 << 28);
    (void)RCC_AHB1ENR;
    PWR_CR |= (3 << 14);

    /**
     * Start the accurate source. The DISC1 carries an 8 MHz crystal;
     * crystal mode (plain HSEON) is correct. If HSERDY never arrives
     * on your board revision, the closk is being fed externally
     * instead: set HSEBYP (bit 18) before HSEON and retry.
     */
    RCC_CR |= (1 << 16);               // HSEON
    while (!(RCC_CR & (1 << 17))) ;    // HSERDY: settle

    /**
     * Geat train. PLL is off out reset; PLLCFGR only writeable
     * while it is off.
     * M=4 (bit 5:0), N=168 (bit 14:6) P=/2 (17:16 = 00),
     * source=HSE (bit 22), Q=7 (27:24)
     */
    RCC_PLLCFGR = (4 << 0)
                | (168 << 6)
                | (0 << 16)    // 00 encodes /2
                | (1 << 22)
                | (7 << 24);

    /**
     * Spin up, wait for clock - the feedback loop settling.
     */
    RCC_CR |= (1 << 24);               // PLLON
    while (!(RCC_CR & (1 << 25))) ;    // PLLRDY

    /**
     * Prepare the flash before the switch: 5 wait states at
     * 168 MHz / 3.3V, plus prefetch + instruction & data cache.
     */
    FLASH_ACR = (1 << 10) | (1 << 9) | (1 << 8) | 5;

    /**
     * Prescalers BEFORE the switch, so APB1/APB2 never see a clock
     * above their ceilings (42 / 90 MHz) even for one cycle.
     * HPRE (7:4) = 0000 -> /1
     * PPRE1 (12:10) = 101 -> /4        PPRE2 (15:13) = 100 -> /2
     */
    RCC_CFGR |= (5u << 10) | (4u << 13);
 
    /**
     * The switch, then insist the hardware confirms.
     */
    RCC_CFGR = (RCC_CFGR & ~3u) | 2u;               /* SW = PLL  */
    while ((RCC_CFGR & (3u << 2)) != (2u << 2)) ;   /* SWS       */
}

/* =================== The 38-pin grind =================== */

/**
 * Every SRAM wire needs the four settings
 *   MODER   = 10 (alternate function)
 *   OSPEEDR = 11 (very high speed - the 11.9 ns edge budget at 84 MHz)
 *   PUPDR   = 00 (no pulls: the bus is actively driver at both ends)
 *   AFR     = 12 (AF12 = FMC)
 */

struct pin {
    char port;
    uint8_t pin;
};

static const struct pin sdram_pins[] = {
    /* NBL0, NBL1        */ {'E',0},  {'E',1},
    /* SDCKE1, SDNE1     */ {'B',5},  {'B',6},
    /* SDNWE             */ {'C',0},
    /* D2, D3            */ {'D',0},  {'D',1},
    /* D13, D14, D15     */ {'D',8},  {'D',9},  {'D',10},
    /* D0, D1            */ {'D',14}, {'D',15},
    /* D4 .. D12         */ {'E',7},  {'E',8},  {'E',9},  {'E',10},
                            {'E',11}, {'E',12}, {'E',13}, {'E',14}, {'E',15},
    /* A0 .. A5          */ {'F',0},  {'F',1},  {'F',2},  {'F',3},
                            {'F',4},  {'F',5},
    /* SDNRAS            */ {'F',11},
    /* A6 .. A9          */ {'F',12}, {'F',13}, {'F',14}, {'F',15},
    /* A10, A11          */ {'G',0},  {'G',1},
    /* BA0, BA1          */ {'G',4},  {'G',5},
    /* SDCLK             */ {'G',8},
    /* SDNCAS            */ {'G',15},
};

#define NPINS (sizeof(sdram_pins) / sizeof(sdram_pins[0]))

static volatile uint32_t* gpio_reg(char port, uint32_t offset)
{
    return (volatile uint32_t *)(0x40020000 + (0x400 * (uint32_t)(port - 'A')) + offset);
}

#define GPIO_MODER_OFFSET   0x00
#define GPIO_OSPEEDR_OFFSET 0x08
#define GPIO_PUPDR_OFFSET   0x0C
#define GPIO_AFRL_OFFSET    0x20
#define GPIO_AFRH_OFFSET    0x24

static void sdram_pins_init(void)
{
    // clocks for GPIOB..GPIOG = AHB1ENR bit 1..6
    RCC_AHB1ENR |= (0x3F << 1);
    (void)RCC_AHB1ENR;

    for (unsigned int i = 0; i < NPINS; ++i)
    {
        char     p = sdram_pins[i].port;
        uint32_t n = sdram_pins[i].pin;

        volatile uint32_t *moder   = gpio_reg(p, GPIO_MODER_OFFSET);
        volatile uint32_t *ospeedr = gpio_reg(p, GPIO_OSPEEDR_OFFSET);
        volatile uint32_t *pupdr   = gpio_reg(p, GPIO_PUPDR_OFFSET);

        *moder   = (*moder   & ~(3 << (n * 2))) | (2 << (n * 2));
        *ospeedr = (*ospeedr | (3 << (n * 2)));
        *pupdr   = (*pupdr   & (3 << (n * 2)));

        {
            /**
             * pins 0-7 live in AFRL, 8-15 in AFRH - the boundary that
             * catches everyone at least once
             */
            uint32_t off = (n < 8) ? GPIO_AFRL_OFFSET : GPIO_AFRH_OFFSET;
            uint32_t sh  = (n & 7) * 4;
            volatile uint32_t *afr = gpio_reg(p, off);
            *afr = (*afr & ~(0xF << sh)) | (12 << sh);
        }
    }
}

/**
 * A permanent addition to your toolkit: print one pin's mux state.
 * You will want this again for the LCD pin swarm in week 10E.
 */
static void dump_pin(char port, uint32_t n)
{
    uint32_t mode  = (*gpio_reg(port, GPIO_MODER_OFFSET)   >> (n * 2)) & 3u;
    uint32_t speed = (*gpio_reg(port, GPIO_OSPEEDR_OFFSET) >> (n * 2)) & 3u;
    uint32_t off   = (n < 8) ? GPIO_AFRL_OFFSET : GPIO_AFRH_OFFSET;
    uint32_t af    = (*gpio_reg(port, off) >> ((n & 7u) * 4u)) & 0xFu;
 
    uart_putc('P');
    uart_putc(port);
    uart_putdec(n);
    uart_puts(": mode=");
    uart_putdec(mode);
    uart_puts(" speed=");
    uart_putdec(speed);
    uart_puts(" af=");
    uart_putdec(af);
    uart_puts(mode == 2 && speed == 3 && af == 12 ? "  OK\n" : "  <-- CHECK\n");
}
 
/* =================== the FMC controller =================== */
 
/*
 * FMC control block. The Discovery wires its SDRAM to BANK 2, which is
 * why the base address is 0xD0000000 (bank 1 would be 0xC0000000).
 */
#define RCC_AHB3ENR    (*(volatile uint32_t *)0x40023838)
#define FMC_SDCR1      (*(volatile uint32_t *)0xA0000140)
#define FMC_SDCR2      (*(volatile uint32_t *)0xA0000144)
#define FMC_SDTR1      (*(volatile uint32_t *)0xA0000148)
#define FMC_SDTR2      (*(volatile uint32_t *)0xA000014C)
#define FMC_SDCMR      (*(volatile uint32_t *)0xA0000150)
#define FMC_SDRTR      (*(volatile uint32_t *)0xA0000154)
#define FMC_SDSR       (*(volatile uint32_t *)0xA0000158)
 
#define SDRAM_BASE     0xD0000000u
#define SDRAM_BYTES    (8u * 1024u * 1024u)
 
/*
 * ---- Derivation A: geometry, into SDCR2 ----
 * IS42S16400J = 4 banks x 4096 rows x 256 columns x 16 bits.
 *   NC   (col bits)  : 256 cols -> 8 bits  -> encoded 8-8   = 0
 *   NR   (row bits)  : 4096 rows -> 12 bits-> encoded 12-11 = 1
 *   MWID             : 16-bit bus          -> 01
 *   NB               : 4 banks             -> 1
 *   CAS              : latency 3           -> 11   (MUST match the mode
 *                                            register written in step 4)
 *   WP               : write protect off   -> 0
 *
 * ---- The bank-2 fine print (RM0090, SDRAM controller section) ----
 * SDCLK, RBURST and RPIPE are taken from SDCR1 -- and TRP, TRC from
 * SDTR1 -- REGARDLESS of which bank the memory is on. So those fields
 * go into the bank-1 registers even though our chip is on bank 2.
 * Get this wrong and the chip never receives a clock, while every
 * register you inspect looks perfectly correct.
 */
#define SDCR_NC_8       (0u << 0)
#define SDCR_NR_12      (1u << 2)
#define SDCR_MWID_16    (1u << 4)
#define SDCR_NB_4       (1u << 6)
#define SDCR_CAS_3      (3u << 7)
#define SDCR_SDCLK_HCLK2 (2u << 10)   /* 84 MHz -> 11.905 ns per cycle */
#define SDCR_RBURST     (1u << 12)
#define SDCR_RPIPE_0    (0u << 13)
 
/*
 * ---- Derivation B: timings @ 11.905 ns/cycle, fields encode (cycles-1) ----
 *   TMRD  ~12 ns -> 2 cycles -> 1     TRC   63 ns -> 6 cycles -> 5  (SDTR1)
 *   TXSR   70 ns -> 6 cycles -> 5     TRP   15 ns -> 2 cycles -> 1  (SDTR1)
 *   TRAS   42 ns -> 4 cycles -> 3     TRCD  15 ns -> 2 cycles -> 1
 *   TWR   ~12 ns -> 2 cycles -> 1
 * Round UP always: a spare cycle costs a hair of bandwidth, a missing
 * one costs correctness intermittently -- the worst currency there is.
 */
#define SDTR_TMRD(x)   (((x) - 1u) << 0)
#define SDTR_TXSR(x)   (((x) - 1u) << 4)
#define SDTR_TRAS(x)   (((x) - 1u) << 8)
#define SDTR_TRC(x)    (((x) - 1u) << 12)   /* SDTR1 only */
#define SDTR_TWR(x)    (((x) - 1u) << 16)
#define SDTR_TRP(x)    (((x) - 1u) << 20)   /* SDTR1 only */
#define SDTR_TRCD(x)   (((x) - 1u) << 24)
 
/*
 * ---- Derivation C: refresh, into SDRTR ----
 * All 4096 rows within 64 ms -> one row every 15.625 us.
 * 15.625 us x 84 MHz = 1312.5 cycles; the manual subtracts a margin of
 * 20 so a refresh request can never collide with an in-flight read.
 *   COUNT = 1292   (field minimum is 41)
 * This is the number whose error gives you memory that works for a few
 * seconds and then rots -- capacitor by capacitor.
 */
#define SDRAM_REFRESH_COUNT  1292u
 
static void fmc_wait_busy(void)
{
    while (FMC_SDSR & (1u << 5)) ;          /* BUSY */
}
 
static void fmc_init(void)
{
    RCC_AHB3ENR |= (1u << 0);               /* FMC clock — a new bus */
    (void)RCC_AHB3ENR;
 
    /* Controller-global fields live in the BANK 1 registers. */
    FMC_SDCR1 = SDCR_SDCLK_HCLK2 | SDCR_RBURST | SDCR_RPIPE_0;
    FMC_SDTR1 = SDTR_TRC(6) | SDTR_TRP(2);
 
    /* Per-bank geometry and timings for our bank 2. */
    FMC_SDCR2 = SDCR_NC_8 | SDCR_NR_12 | SDCR_MWID_16
              | SDCR_NB_4 | SDCR_CAS_3;
    FMC_SDTR2 = SDTR_TMRD(2) | SDTR_TXSR(6) | SDTR_TRAS(4)
              | SDTR_TWR(2)  | SDTR_TRCD(2);
}
 
/* Print the programmed registers so you can decode them by hand against
   your derivation table -- the five-minute audit that catches a
   bank-1/bank-2 mix-up before it becomes a mystery. */
static void fmc_dump(void)
{
    uart_puts("SDCR1=0x"); uart_puthex(FMC_SDCR1);
    uart_puts("  SDCR2=0x"); uart_puthex(FMC_SDCR2);
    uart_puts("\n SDTR1=0x"); uart_puthex(FMC_SDTR1);
    uart_puts("  SDTR2=0x"); uart_puthex(FMC_SDTR2);
    uart_puts("\n SDRTR=0x"); uart_puthex(FMC_SDRTR);
    uart_puts("\n");
}


/* =================== step 4: the wake-up liturgy =================== */
 
/*
 * SDCMR is a doorbell, not a settings register: composing a command in
 * its fields and writing it ISSUES that command on the NRAS/NCAS/SDNWE
 * wires. Poll SDSR.BUSY between rings -- the controller needs the bus
 * back before it will accept another.
 *
 * The sequence below is JEDEC's, obeyed by every SDRAM ever made:
 *   1. clock config enable, then wait >= 100 us  (chip settling)
 *   2. precharge all                              (close every drawer)
 *   3. auto-refresh x 8                           (standard warm-up)
 *   4. load mode register                         (program the CHIP)
 *   5. start the refresh timer                    (hire the rider)
 */
#define SDCMR_MODE_NORMAL     0u
#define SDCMR_MODE_CLK_EN     1u
#define SDCMR_MODE_PALL       2u
#define SDCMR_MODE_AUTOREF    3u
#define SDCMR_MODE_LOAD_MR    4u
#define SDCMR_CTB2            (1u << 3)
#define SDCMR_CTB1            (1u << 4)
#define SDCMR_NRFS(n)         (((n) - 1u) << 5)
#define SDCMR_MRD(x)          ((x) << 9)
 
/*
 * Mode register payload, from the IS42S16400J mode-register diagram:
 *   burst length 1        : bits 2:0 = 000
 *   burst type sequential : bit  3   = 0
 *   CAS latency 3         : bits 6:4 = 011   <-- MUST match SDCR2.CAS
 *   operating mode std    : bits 8:7 = 00
 *   single-location write : bit  9   = 1
 *   => 0x030 | 0x200 = 0x230
 */
#define SDRAM_MODEREG   0x230u
 
/* Busy-wait microseconds from the 168 MHz core clock. Crude but exact
   enough for a >= 100 us settling requirement (we spend ~1 ms). */
static void delay_us(uint32_t us)
{
    volatile uint32_t n = us * (SYSCLK_HZ / 1000000u) / 4u;
    while (n--) ;
}
 
static void sdram_wakeup(void)
{
    /* 1. clock configuration enable + settling */
    fmc_wait_busy();
    FMC_SDCMR = SDCMR_MODE_CLK_EN | SDCMR_CTB2;
    delay_us(1000);                       /* >= 100 us; we give 1 ms */
 
    /* 2. precharge all: close every row in every bank */
    fmc_wait_busy();
    FMC_SDCMR = SDCMR_MODE_PALL | SDCMR_CTB2;
 
    /* 3. eight auto-refresh cycles: the standard's warm-up */
    fmc_wait_busy();
    FMC_SDCMR = SDCMR_MODE_AUTOREF | SDCMR_CTB2 | SDCMR_NRFS(8);
 
    /* 4. load mode register: configure the CHIP itself, through its
          own address pins. CAS here must equal SDCR2's CAS field. */
    fmc_wait_busy();
    FMC_SDCMR = SDCMR_MODE_LOAD_MR | SDCMR_CTB2 | SDCMR_MRD(SDRAM_MODEREG);
 
    /* 5. hire the refresh rider -- only now that the chip is awake and
          listening. Wrong value here = memory that works for seconds,
          then rots. */
    fmc_wait_busy();
    FMC_SDRTR = (SDRAM_REFRESH_COUNT << 1);
 
    fmc_wait_busy();
}
 
/* First proof of life: a handful of words, written and read back. The
   real test suite (address-in-address, walking ones, byte lanes) is
   step 5 -- and note that a write-then-immediately-verify test CANNOT
   detect a refresh bug, because a freshly written cell is fully
   charged. Step 5 inserts a wait for exactly that reason. */
static int sdram_smoke_test(void)
{
    volatile uint32_t *m = (volatile uint32_t *)SDRAM_BASE;
    const uint32_t pattern[4] = { 0xDEADBEEFu, 0x00000000u,
                                  0xFFFFFFFFu, 0xA5A5A5A5u };
    int ok = 1;
 
    for (int i = 0; i < 4; i++)
        m[i] = pattern[i];
 
    for (int i = 0; i < 4; i++) {
        uart_puts("  [");
        uart_puthex((uint32_t)&m[i]);
        uart_puts("] wrote 0x");
        uart_puthex(pattern[i]);
        uart_puts("  read 0x");
        uart_puthex(m[i]);
        if (m[i] != pattern[i]) {
            uart_puts("   MISMATCH");
            ok = 0;
        }
        uart_puts("\n");
    }
 
    /* one word at the far end: proves the address lines reach 8 MB */
    volatile uint32_t *far = (volatile uint32_t *)(SDRAM_BASE + SDRAM_BYTES - 4u);
    *far = 0x12345678u;
    uart_puts("  [");
    uart_puthex((uint32_t)far);
    uart_puts("] wrote 0x12345678  read 0x");
    uart_puthex(*far);
    if (*far != 0x12345678u) {
        uart_puts("   MISMATCH");
        ok = 0;
    }
    uart_puts("\n");
 
    /* and re-read word 0: if writing the far end corrupted it, an
       address line is stuck or swapped (aliasing). */
    if (m[0] != pattern[0]) {
        uart_puts("  ALIASING: word 0 changed after far write\n");
        ok = 0;
    }
 
    return ok;
}

/*
 * One pass per fault class. A single big loop with a constant pattern
 * would "pass" while an address line is dead -- you would be reading
 * the right value from the wrong cell and never know.
 */
 
#define SDRAM_WORDS   (SDRAM_BYTES / 4u)
 
static void report_fail(const char *what, uint32_t addr,
                        uint32_t expect, uint32_t got)
{
    uart_puts("  FAIL ");
    uart_puts(what);
    uart_puts(" @0x");   uart_puthex(addr);
    uart_puts(" exp 0x"); uart_puthex(expect);
    uart_puts(" got 0x"); uart_puthex(got);
    uart_puts(" xor 0x"); uart_puthex(expect ^ got);
    uart_puts("\n");
}

/*
 * Pass 1 -- address-in-address.
 * Each cell holds its own address, so any misrouting produces a
 * mismatch that NAMES the fault: the XOR of expected and actual has
 * bits set exactly where the guilty address lines are.
 */
static int test_addr_in_addr(void)
{
    volatile uint32_t *m = (volatile uint32_t *)SDRAM_BASE;
    uint32_t i;
 
    uart_puts(" pass 1: address-in-address, writing");
    for (i = 0; i < SDRAM_WORDS; i++) {
        m[i] = (uint32_t)&m[i];
        if ((i & 0x3FFFFu) == 0)
            uart_putc('.');
    }
 
    uart_puts(" verifying");
    for (i = 0; i < SDRAM_WORDS; i++) {
        uint32_t got = m[i];
        uint32_t exp = (uint32_t)&m[i];
        if (got != exp) {
            uart_puts("\n");
            report_fail("addr", exp, exp, got);
            return 0;
        }
        if ((i & 0x3FFFFu) == 0)
            uart_putc('.');
    }
    uart_puts(" OK\n");
    return 1;
}

/*
 * Pass 2 -- walking ones (and their inverses).
 * Isolates DATA lines: a shorted pair shows two bits always agreeing,
 * an open line shows a bit stuck. Sampled addresses only -- 32 bits x
 * 2 polarities is enough per location.
 */
static int test_walking_ones(void)
{
    const uint32_t spots[] = { 0u, 1u, 0x1000u, 0x40000u, 0x80000u,
                               SDRAM_WORDS / 2u, SDRAM_WORDS - 1u };
    uart_puts(" pass 2: walking ones");
 
    for (unsigned s = 0; s < sizeof spots / sizeof spots[0]; s++) {
        volatile uint32_t *p = (volatile uint32_t *)SDRAM_BASE + spots[s];
 
        for (int bit = 0; bit < 32; bit++) {
            uint32_t v = 1u << bit;
 
            *p = v;
            if (*p != v) {
                uart_puts("\n");
                report_fail("walk1", (uint32_t)p, v, *p);
                return 0;
            }
 
            *p = ~v;
            if (*p != ~v) {
                uart_puts("\n");
                report_fail("walk0", (uint32_t)p, ~v, *p);
                return 0;
            }
        }
        uart_putc('.');
    }
    uart_puts(" OK\n");
    return 1;
}


/*
 * Pass 3 -- byte lanes.
 * The ONLY pass that exercises NBL0/NBL1. If byte writes silently
 * broadcast to the whole word, you find out here -- not months later
 * as inexplicable framebuffer corruption.
 */
static int test_byte_lanes(void)
{
    volatile uint32_t *w  = (volatile uint32_t *)(SDRAM_BASE + 0x100u);
    volatile uint8_t  *b  = (volatile uint8_t  *)w;
    volatile uint16_t *h  = (volatile uint16_t *)w;
 
    uart_puts(" pass 3: byte lanes");
 
    *w = 0x00000000u;
    b[0] = 0xAAu;
    if (*w != 0x000000AAu) {
        uart_puts("\n"); report_fail("byte0", (uint32_t)w, 0x000000AAu, *w); return 0;
    }
    b[3] = 0xBBu;
    if (*w != 0xBB0000AAu) {
        uart_puts("\n"); report_fail("byte3", (uint32_t)w, 0xBB0000AAu, *w); return 0;
    }
 
    *w = 0xFFFFFFFFu;
    h[0] = 0x1234u;
    if (*w != 0xFFFF1234u) {
        uart_puts("\n"); report_fail("half0", (uint32_t)w, 0xFFFF1234u, *w); return 0;
    }
    h[1] = 0x5678u;
    if (*w != 0x56781234u) {
        uart_puts("\n"); report_fail("half1", (uint32_t)w, 0x56781234u, *w); return 0;
    }
 
    uart_puts(" OK\n");
    return 1;
}
 
/*
 * Pass 4 -- decay.
 * The only pass that can detect a refresh bug, because a freshly
 * written cell is fully charged: write, WAIT, then verify without
 * rewriting. Raise SDRAM_REFRESH_COUNT 50x and watch the error count
 * climb with the wait -- entropy, on demand.
 */
static int test_decay(uint32_t wait_ms)
{
    volatile uint32_t *m = (volatile uint32_t *)SDRAM_BASE;
    const uint32_t region = 64u * 1024u;      /* 64 K words = 256 KB */
    uint32_t errors = 0;
 
    uart_puts(" pass 4: decay (");
    uart_putdec(wait_ms);
    uart_puts(" ms wait)");
 
    for (uint32_t i = 0; i < region; i++)
        m[i] = 0x5A5A0000u | (i & 0xFFFFu);
 
    {   /* wait WITHOUT touching the region: only refresh keeps it alive */
        uint32_t start = millis();
        while (millis() - start < wait_ms) ;
    }
 
    for (uint32_t i = 0; i < region; i++) {
        uint32_t exp = 0x5A5A0000u | (i & 0xFFFFu);
        if (m[i] != exp) {
            if (errors == 0) {
                uart_puts("\n");
                report_fail("decay", (uint32_t)&m[i], exp, m[i]);
            }
            errors++;
        }
    }
 
    if (errors) {
        uart_puts("  total decayed words: ");
        uart_putdec(errors);
        uart_puts("\n");
        return 0;
    }
    uart_puts(" OK\n");
    return 1;
}
 
__attribute__((section(".sdram"))) uint16_t frameBuffer[240 * 320];

static int sdram_test_buffer(uint32_t size)
{
    for (int i = 0; i < size; ++i)
        frameBuffer[i] = i * i;
    
    for (int i = 0; i < size; ++i)
        if (frameBuffer[i] != i * i) {
            uart_puts(" pass 5: RW to frameBuffer... FAIL\n");
            return 0;
        }

    uart_puts(" pass 5: RW to frameBuffer... OK\n");
    return 1;
}

/* Stretch: how much slower is SDRAM than internal SRAM? */
static void bench_memset(void)
{
    volatile uint32_t *sd = (volatile uint32_t *)SDRAM_BASE;
    static uint32_t    sr[8192];              /* 32 KB in internal SRAM */
    uint32_t t0, t_sram, t_sdram;
    const uint32_t n = 8192u;
    const uint32_t reps = 32u;                /* 1 MB total each */
 
    t0 = millis();
    for (uint32_t r = 0; r < reps; r++)
        for (uint32_t i = 0; i < n; i++)
            sr[i] = i * i;
    t_sram = millis() - t0;
 
    t0 = millis();
    for (uint32_t r = 0; r < reps; r++)
        for (uint32_t i = 0; i < n; i++)
            sd[i] = i * i;
    t_sdram = millis() - t0;
 
    uart_puts(" bench 1 MB write: SRAM ");
    uart_putdec(t_sram);
    uart_puts(" ms, SDRAM ");
    uart_putdec(t_sdram);
    uart_puts(" ms\n");
}

static void sdram_test_all(void)
{
    int ok = 1;
    uart_puts("SDRAM test suite (8 MB):\n");
    ok &= test_addr_in_addr();
    ok &= test_walking_ones();
    ok &= test_byte_lanes();
    ok &= test_decay(1000u);
    ok &= sdram_test_buffer(256);
    bench_memset();
    uart_puts(ok ? "SDRAM: ALL PASSES OK -- 8388608 bytes good\n\n"
                 : "SDRAM: FAILURES above -- see the fault decoder\n\n");
}

/* =================== main =================== */

int main(void)
{
    clock_init_168mhz();

    uart_init();
    systick_init();
    spi5_init();
    button_init();

    RCC_AHB1ENR |= (1 << 6);         /* GPIOG clock */
    (void)RCC_AHB1ENR;
    GPIOG_MODER &= ~((3u << (13 * 2)) | (3u << (14 * 2)));
    GPIOG_MODER |=  ((1u << (13 * 2)) | (1u << (14 * 2)));

    uart_puts("\n=== week09e step 1: PLL ===\n");
    uart_puts("SYSCLK: ");  uart_putdec(SYSCLK_HZ);  uart_puts(" Hz (PLL)\n");
    
    uart_puts("SWS says: "); uart_putdec((RCC_CFGR >> 2) & 3); // Must print 2 = PLL
    uart_puts(" (2 = PLL)\n");
    
    uart_puts("blink_delay @ ");  uart_puthex((uint32_t) &blink_delay);
    uart_puts(" = ");  uart_puthex((uint32_t) blink_delay); uart_puts("\n");

    /* ---- step 2: mux the 38 SDRAM pins, then spot-check ---- */
    sdram_pins_init();
 
    uart_puts("\npin spot-check (both AFR halves, three ports):\n");
    for (uint32_t i = 0; i < NPINS; ++i)
        dump_pin(sdram_pins[i].port, sdram_pins[i].pin);
 
    /* ---- step 3: program the controller, then audit it ---- */
    fmc_init();
    uart_puts("FMC programmed:\n ");
    fmc_dump();
    uart_puts("(decode each field by hand against your table)\n");
    
    /* ---- step 4: wake the chip, then prove it lives ---- */
    sdram_wakeup();
    uart_puts("SDRAM woken. smoke test @ 0xD0000000:\n");
    if (sdram_smoke_test())
        uart_puts("SDRAM: smoke test PASSED -- 8 MB now exists.\n\n");
    else
        uart_puts("SDRAM: smoke test FAILED -- see the decoder in the brief.\n\n");

    /* ---- step 5: the real suite ---- */
    sdram_test_all();

    /* Initialize all tasks here */
    task_init(0, task_green);
    task_init(1, task_red);
    task_init(2, task_uart);
    // task_init(3, task_gyro);

    /* layer 6: PendSV to the lowest priority — 'late' is the feature */
    SCB_SHPR3 |= (0xFFu << 16);

    current = 0;
    sched_started = 1;

    /* hand the CPU to task 0 on its own stack. main's frames up on MSP
       are abandoned from here on; MSP becomes handler territory only.  */
    start_first_task((uint32_t)&stacks[0][STACK_SIZE], task_green);

    for (;;) ;                                /* never reached            */
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
    pendsv_handler,     /* 14: PendSV  (week 7's star)   */
    systick_handler,    /* 15: SysTick                   */
    0, 0, 0, 0, 0, 0,   /* 16-21: IRQ 0..5               */
    exti0_handler,      /* 22: IRQ 6 = EXTI0             */
};