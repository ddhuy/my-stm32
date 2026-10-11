#include <stdbool.h>
#include <stdint.h>

#include "soc/stm32f429/rcc_regs.h"
#include "soc/stm32f429/gpio_regs.h"
#include "soc/stm32f429/fmc_regs.h"
#include "drivers/ram/stm32_sdram.h"

#include "board/stm32f429i-disc1/board.h"
#include "board/stm32f429i-disc1/board_internal.h"


/* IS42S16400J-7: 4 banks x 4096 rows x 256 columns x 16 bit, on FMC bank 2 */
#define SDRAM_ROWS               4096u
#define SDRAM_REFRESH_PERIOD_MS  64u    /* every row refreshed within 64 ms */
#define SDRAM_SDCLK_HZ           (CLOCK_HCLK_HZ / 2u)   /* FMC_SDCR_SDCLK_HCLK2 */

static const stm32_sdram_config_t sdram_config = {
    .bank           = 2u,
    .sdclk          = FMC_SDCR_SDCLK_HCLK2,
    .read_burst     = true,
    .read_pipe      = 0u,

    .column_bits    = 8u,
    .row_bits       = 12u,
    .bus_width      = FMC_SDCR_MWID_16,
    .internal_banks = 4u,
    .cas_latency    = 3u,

    /* SDCLK cycles, 11.9 ns each at 84 MHz */
    .tmrd           = 2u,   /* load mode to active          */
    .txsr           = 7u,   /* exit self-refresh,  >= 70 ns */
    .tras           = 4u,   /* self-refresh time,  >= 42 ns */
    .trc            = 7u,   /* row cycle,          >= 63 ns */
    .twr            = 2u,   /* write recovery               */
    .trp            = 3u,   /* row precharge,      >= 15 ns */
    .trcd           = 2u,   /* row to column,      >= 15 ns */

    .refresh_count  = STM32_SDRAM_REFRESH_COUNT(SDRAM_REFRESH_PERIOD_MS, SDRAM_ROWS,
                                                SDRAM_SDCLK_HZ),
    /* Each loop takes at least one cycle, so this is >= 1 ms (>= 100 us needed) */
    .powerup_delay_loops = CLOCK_HCLK_HZ / 1000u,
};

_Static_assert(STM32_SDRAM_REFRESH_COUNT(SDRAM_REFRESH_PERIOD_MS, SDRAM_ROWS, SDRAM_SDCLK_HZ) > 41u,
               "SDRAM refresh count out of range");


typedef struct pin
{
    uint8_t port;   /* GPIO_PORT_A .. GPIO_PORT_K */
    uint8_t n;
} sdram_pin_t;

/* Discovery SDRAM routing (FMC, AF12) */
static const sdram_pin_t pins[] = {
    /* NBL0, NBL1 */
    {GPIO_PORT_E, 0}, {GPIO_PORT_E, 1},
    /* SDCKE1, SDNE1 (bank 2 control) */
    {GPIO_PORT_B, 5}, {GPIO_PORT_B, 6},
    /* SDNWE */
    {GPIO_PORT_C, 0},
    /* D2, D3, D13, D14, D15, D0, D1 */
    {GPIO_PORT_D, 0}, {GPIO_PORT_D, 1}, {GPIO_PORT_D, 8}, {GPIO_PORT_D, 9},
    {GPIO_PORT_D, 10}, {GPIO_PORT_D, 14}, {GPIO_PORT_D, 15},
    /* D4 .. D12 */
    {GPIO_PORT_E, 7}, {GPIO_PORT_E, 8}, {GPIO_PORT_E, 9}, {GPIO_PORT_E, 10},
    {GPIO_PORT_E, 11}, {GPIO_PORT_E, 12}, {GPIO_PORT_E, 13}, {GPIO_PORT_E, 14},
    {GPIO_PORT_E, 15},
    /* A0 .. A5, SDNRAS, A6 .. A9 */
    {GPIO_PORT_F, 0}, {GPIO_PORT_F, 1}, {GPIO_PORT_F, 2}, {GPIO_PORT_F, 3},
    {GPIO_PORT_F, 4}, {GPIO_PORT_F, 5}, {GPIO_PORT_F, 11}, {GPIO_PORT_F, 12},
    {GPIO_PORT_F, 13}, {GPIO_PORT_F, 14}, {GPIO_PORT_F, 15},
    /* A10, A11, BA0, BA1, SDCLK, SDNCAS */
    {GPIO_PORT_G, 0}, {GPIO_PORT_G, 1}, {GPIO_PORT_G, 4}, {GPIO_PORT_G, 5},
    {GPIO_PORT_G, 8}, {GPIO_PORT_G, 15}
};


static void sdram_pins_init(void)
{
    RCC_AHB1ENR |= RCC_AHB1ENR_GPIOBEN | RCC_AHB1ENR_GPIOCEN | RCC_AHB1ENR_GPIODEN |
                   RCC_AHB1ENR_GPIOEEN | RCC_AHB1ENR_GPIOFEN | RCC_AHB1ENR_GPIOGEN;
    (void)RCC_AHB1ENR;

    for (unsigned i = 0; i < sizeof(pins) / sizeof(pins[0]); ++i)
    {
        board_pin_af(GPIO_PORT_BASE(pins[i].port), pins[i].n,
                     GPIO_AF_FMC, GPIO_SPEED_VERY_HIGH, GPIO_PULL_NONE);
    }
}

bool board_sdram_init(void)
{
    sdram_pins_init();

    RCC_AHB3ENR |= RCC_AHB3ENR_FMCEN;
    (void)RCC_AHB3ENR;

    return stm32_sdram_init(&sdram_config);
}
