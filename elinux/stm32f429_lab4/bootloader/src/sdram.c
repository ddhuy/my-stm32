#include <stdint.h>
#include <stdbool.h>

#include "stm32f429/rcc_regs.h"
#include "stm32f429/gpio_regs.h"
#include "stm32f429/fmc_regs.h"
#include "stm32f429/mpu_regs.h"

#include "sdram.h"


#define FMC_TIMEOUT 1000000u


typedef struct pin
{
    char port;
    uint8_t n;
} sdram_pin_t;


/* Discovery SDRAM routing: same pin table as your Lab 2. */
static const sdram_pin_t pins[] = {
    {'E', 0}, {'E', 1},
    {'B', 5}, {'B', 6},
    {'C', 0},
    {'D', 0}, {'D', 1}, {'D', 8}, {'D', 9}, {'D', 10}, {'D', 14}, {'D', 15},
    {'E', 7}, {'E', 8}, {'E', 9}, {'E', 10}, {'E', 11}, {'E', 12}, {'E', 13},
    {'E', 14}, {'E', 15},
    {'F', 0}, {'F', 1}, {'F', 2}, {'F', 3}, {'F', 4}, {'F', 5}, {'F', 11},
    {'F', 12}, {'F', 13}, {'F', 14}, {'F', 15},
    {'G', 0}, {'G', 1}, {'G', 4}, {'G', 5}, {'G', 8}, {'G', 15}
};

static inline void dsb(void)
{
    __asm volatile("dsb 0xF" ::: "memory");
}

static inline void isb(void)
{
    __asm volatile("isb 0xF" ::: "memory");
}

static void spin(unsigned n)
{
    while (n--) {
        __asm volatile("nop");
    }
}

static volatile uint32_t *gpio_reg(char p, uint32_t offset)
{
    return (volatile uint32_t *)(uintptr_t)(0x40020000u + 0x400u * (uint32_t)(p - 'A') + offset);
}

static void pins_sdram_init(void)
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

static bool fmc_wait(void)
{
    uint32_t timeout = FMC_TIMEOUT; 

    while (FMC_SDSR & FMC_SDSR_BUSY)
    {
        if (--timeout == 0u) {
            return false;
        }
    }

    return true;
}

static bool fmc_cmd(uint32_t cmd)
{
    if (!fmc_wait())
        return false;
    
        FMC_SDCMR = cmd | FMC_CMD_CTB2;

    if (!fmc_wait())
        return false;

    return true;
}

static bool fmc_sdram_init(void)
{
    RCC_AHB3ENR |= RCC_AHB3ENR_FMCEN;
    (void)RCC_AHB3ENR;

    /* Bank-2 SDRAM: shared SDCLK/RBURST/RPIPE in SDCR1; TRC/TRP in SDTR1. */
    FMC_SDCR1 = (2u << 10) | (1u << 12); /* SDCLK=HCLK/2, read burst */
    FMC_SDCR2 = (1u << 2) | (1u << 4) | (1u << 6) | (3u << 7);
    FMC_SDTR1 = ((7u - 1u) << 12) | ((3u - 1u) << 20);
    FMC_SDTR2 = ((2u - 1u) << 0) | ((7u - 1u) << 4) | ((4u - 1u) << 8) |
                ((2u - 1u) << 16) | ((2u - 1u) << 24);

    if (!fmc_cmd(FMC_CMD_CLK_ENABLE))  /* clock configuration enable, bank 2 */
        return false;

    spin(168000u);  /* conservative >100us at 168MHz */

    if (!fmc_cmd(FMC_CMD_PALL))  /* precharge all */
        return false;

    if (!fmc_cmd(FMC_CMD_AUTOREFRESH | ((8u - 1u) << 5)))  /* 8 auto-refresh cycles */
        return false;

    if (!fmc_cmd(FMC_CMD_LOAD_MODE | (0x230u << 9)))  /* burst 1, CAS3, single write */
        return false;
    
    FMC_SDRTR = 1292u << 1;

    return fmc_wait();
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
    MPU_RASR = MPU_RASR_AP_FULL |
               MPU_RASR_TEX_NORMAL |
               MPU_RASR_SIZE_8MB |
               MPU_RASR_ENABLE;
    MPU_CTRL = MPU_CTRL_PRIVDEFENA | MPU_CTRL_ENABLE;
    dsb();
    isb();
}

bool sdram_init(void)
{   
    pins_sdram_init();

    if (!fmc_sdram_init())
        return false;

    mpu_sdram_exec();

    return true;
}