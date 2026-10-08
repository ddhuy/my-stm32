#include <stdint.h>
#include <stddef.h>

#define REG(a) (*(volatile uint32_t *)(a))
#define RCC_CR REG(0x40023800u)
#define RCC_PLLCFGR REG(0x40023804u)
#define RCC_CFGR REG(0x40023808u)
#define RCC_AHB1ENR REG(0x40023830u)
#define RCC_AHB3ENR REG(0x40023838u)
#define RCC_APB1ENR REG(0x40023840u)
#define PWR_CR REG(0x40007000u)
#define FLASH_ACR REG(0x40023C00u)
#define FMC_SDCR1 REG(0xA0000140u)
#define FMC_SDCR2 REG(0xA0000144u)
#define FMC_SDTR1 REG(0xA0000148u)
#define FMC_SDTR2 REG(0xA000014Cu)
#define FMC_SDCMR REG(0xA0000150u)
#define FMC_SDRTR REG(0xA0000154u)
#define FMC_SDSR REG(0xA0000158u)
#define MPU_TYPE REG(0xE000ED90u)
#define MPU_CTRL REG(0xE000ED94u)
#define MPU_RNR REG(0xE000ED98u)
#define MPU_RBAR REG(0xE000ED9Cu)
#define MPU_RASR REG(0xE000EDA0u)
#define SCB_CFSR REG(0xE000ED28u)
#define SCB_HFSR REG(0xE000ED2Cu)
#define SCB_MMFAR REG(0xE000ED34u)
#define SCB_BFAR REG(0xE000ED38u)
#define SCB_SHCSR REG(0xE000ED24u)
#define GPIOG_MODER REG(0x40021800u)
#define GPIOG_BSRR REG(0x40021818u)

extern uint8_t __sdram_text_load__, __sdram_text_start__, __sdram_text_end__;
volatile uint32_t lab3_result;
volatile uint32_t lab3_pc;
volatile uint32_t lab3_calls;
volatile uint32_t fault_cfsr, fault_hfsr, fault_mmfar, fault_bfar;

static inline void dsb(void) { __asm volatile("dsb 0xF" ::: "memory"); }
static inline void isb(void) { __asm volatile("isb 0xF" ::: "memory"); }
static void spin(unsigned n)
{
    while (n--)
        __asm volatile("nop");
}

/* HSI=16 MHz, PLLM=8, PLLN=168, PLLP=2 => SYSCLK=168 MHz. */
static void clock_init(void)
{
    RCC_APB1ENR |= (1u << 28);
    (void)RCC_APB1ENR;
    PWR_CR = (PWR_CR & ~(3u << 14)) | (3u << 14);
    FLASH_ACR = (1u << 10) | (1u << 9) | (1u << 8) | 5u;
    while ((FLASH_ACR & 7u) != 5u)
    {
    }
    RCC_CFGR = (RCC_CFGR & ~((15u << 4) | (7u << 10) | (7u << 13))) |
               (5u << 10) | (4u << 13);
    RCC_CR &= ~(1u << 24);
    while (RCC_CR & (1u << 25))
    {
    }
    RCC_PLLCFGR = (8u << 0) | (168u << 6) | (7u << 24); /* HSI PLL source */
    RCC_CR |= (1u << 24);
    while (!(RCC_CR & (1u << 25)))
    {
    }
    RCC_CFGR = (RCC_CFGR & ~3u) | 2u;
    while (((RCC_CFGR >> 2) & 3u) != 2u)
    {
    }
}

struct pin
{
    char port;
    uint8_t n;
};
/* Discovery SDRAM routing: same pin table as your Lab 2. */
static const struct pin pins[] = {
    {'E', 0}, {'E', 1}, {'B', 5}, {'B', 6}, {'C', 0}, {'D', 0}, {'D', 1}, {'D', 8}, {'D', 9}, {'D', 10}, {'D', 14}, {'D', 15}, {'E', 7}, {'E', 8}, {'E', 9}, {'E', 10}, {'E', 11}, {'E', 12}, {'E', 13}, {'E', 14}, {'E', 15}, {'F', 0}, {'F', 1}, {'F', 2}, {'F', 3}, {'F', 4}, {'F', 5}, {'F', 11}, {'F', 12}, {'F', 13}, {'F', 14}, {'F', 15}, {'G', 0}, {'G', 1}, {'G', 4}, {'G', 5}, {'G', 8}, {'G', 15}};
static volatile uint32_t *gpio_reg(char p, uint32_t offset)
{
    return (volatile uint32_t *)(uintptr_t)(0x40020000u + 0x400u * (uint32_t)(p - 'A') + offset);
}
static void pins_init(void)
{
    RCC_AHB1ENR |= (0x3Fu << 1);
    (void)RCC_AHB1ENR;
    for (unsigned i = 0; i < sizeof(pins) / sizeof(pins[0]); ++i)
    {
        char p = pins[i].port;
        unsigned n = pins[i].n;
        unsigned sh = n * 2u;
        volatile uint32_t *moder = gpio_reg(p, 0), *otype = gpio_reg(p, 4);
        volatile uint32_t *speed = gpio_reg(p, 8), *pupd = gpio_reg(p, 12);
        volatile uint32_t *afr = gpio_reg(p, n < 8 ? 0x20u : 0x24u);
        unsigned afsh = (n & 7u) * 4u;
        *moder = (*moder & ~(3u << sh)) | (2u << sh);
        *otype &= ~(1u << n);
        *speed = (*speed & ~(3u << sh)) | (3u << sh);
        *pupd &= ~(3u << sh);
        *afr = (*afr & ~(15u << afsh)) | (12u << afsh);
    }
}
static void fmc_wait(void)
{
    while (FMC_SDSR & (1u << 5))
    {
    }
}
static void fmc_cmd(uint32_t cmd)
{
    fmc_wait();
    FMC_SDCMR = cmd | (1u << 3);
    fmc_wait();
}
static void sdram_init(void)
{
    RCC_AHB3ENR |= 1u;
    (void)RCC_AHB3ENR;
    /* Bank-2 SDRAM: shared SDCLK/RBURST/RPIPE in SDCR1; TRC/TRP in SDTR1. */
    FMC_SDCR1 = (2u << 10) | (1u << 12); /* SDCLK=HCLK/2, read burst */
    FMC_SDCR2 = (1u << 2) | (1u << 4) | (1u << 6) | (3u << 7);
    FMC_SDTR1 = ((7u - 1u) << 12) | ((3u - 1u) << 20);
    FMC_SDTR2 = ((2u - 1u) << 0) | ((7u - 1u) << 4) | ((4u - 1u) << 8) |
                ((2u - 1u) << 16) | ((2u - 1u) << 24);
    fmc_cmd(1u);                    /* clock configuration enable, bank 2 */
    spin(168000u);                  /* conservative >100us at 168MHz */
    fmc_cmd(2u);                    /* precharge all */
    fmc_cmd(3u | ((8u - 1u) << 5)); /* 8 auto-refresh cycles */
    fmc_cmd(4u | (0x230u << 9));    /* burst 1, CAS3, single write */
    FMC_SDRTR = 1292u << 1;
    fmc_wait();
}

/* Cortex-M4 external SDRAM region needs MPU Normal-memory override for fetch. */
static void mpu_sdram_exec(void)
{
    MPU_CTRL = 0;
    dsb();
    isb();
    MPU_RNR = 0;
    MPU_RBAR = 0xD0000000u;
    /* XN=0, AP=011 full access, TEX=001 Normal non-cacheable,
       S/C/B=0, SIZE=22 => 2^(22+1)=8 MiB, region enabled. */
    MPU_RASR = (3u << 24) | (1u << 19) | (22u << 1) | 1u;
    MPU_CTRL = (1u << 2) | 1u; /* PRIVDEFENA | ENABLE */
    dsb();
    isb();
}

static void relocate(void)
{
    const uint8_t *src = &__sdram_text_load__;
    uint8_t *dst = &__sdram_text_start__;
    uint8_t *end = &__sdram_text_end__;
    while (dst < end)
        *dst++ = *src++;
    dsb();
    isb();
}

/* Self-contained relocated function: no literal references to Flash helpers. */
__attribute__((section(".sdram_text"), noinline, used))
uint32_t
sdram_add(uint32_t a, uint32_t b)
{
    __asm volatile("nop");
    return (a ^ 0x13579BDFu) + b;
}

void fault_handler(void)
{
    fault_cfsr = SCB_CFSR;
    fault_hfsr = SCB_HFSR;
    fault_mmfar = SCB_MMFAR;
    fault_bfar = SCB_BFAR;
    for (;;)
        __asm volatile("bkpt #0");
}

int main(void)
{
    clock_init();
    pins_init();
    sdram_init();
    /* LED PG13 is not among SDRAM pins. */
    RCC_AHB1ENR |= (1u << 6);
    (void)RCC_AHB1ENR;
    GPIOG_MODER = (GPIOG_MODER & ~(3u << 26)) | (1u << 26);
    SCB_SHCSR |= (1u << 16) | (1u << 17) | (1u << 18); /* fault handlers */
    mpu_sdram_exec();
    relocate();
    lab3_pc = (uint32_t)(uintptr_t)&sdram_add;
    lab3_result = sdram_add(0x12345678u, 0x11111111u);
    lab3_calls++;
    if (lab3_result == ((0x12345678u ^ 0x13579BDFu) + 0x11111111u))
        GPIOG_BSRR = (1u << 13); /* green LED on */
    else
        GPIOG_BSRR = (1u << (13 + 16));
    for (;;)
        __asm volatile("nop");
}
