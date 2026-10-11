#ifndef STM32_SDRAM_H
#define STM32_SDRAM_H

#include <stdbool.h>
#include <stdint.h>

/*
 * SDRAM controller driver for the STM32 FMC.
 * The board supplies the chip geometry and timings, routes the pins and
 * enables the FMC clock (RCC_AHB3ENR_FMCEN) before calling stm32_sdram_init().
 */

/*
 * Refresh timer value: (refresh period / rows) * SDCLK - 20 margin (RM0090 37.7.5).
 * Must end up between 42 and 0x1FFF.
 */
#define STM32_SDRAM_REFRESH_COUNT(period_ms, rows, sdclk_hz) \
    ((((period_ms) * ((sdclk_hz) / 1000u)) / (rows)) - 20u)

typedef struct
{
    uint32_t bank;            /* 1 = SDNE0/SDCKE0 (0xC0000000), 2 = SDNE1/SDCKE1 (0xD0000000) */

    /* Controller (shared by both banks) */
    uint32_t sdclk;           /* FMC_SDCR_SDCLK_HCLK2 or FMC_SDCR_SDCLK_HCLK3 */
    bool     read_burst;
    uint32_t read_pipe;       /* extra HCLK delay when reading, 0..2 */

    /* Chip geometry */
    uint32_t column_bits;     /* 8..11  */
    uint32_t row_bits;        /* 11..13 */
    uint32_t bus_width;       /* FMC_SDCR_MWID_8 / _16 / _32 */
    uint32_t internal_banks;  /* 2 or 4 */
    uint32_t cas_latency;     /* 1..3   */

    /* Timings in SDCLK cycles (1..16), from the chip datasheet */
    uint32_t tmrd;            /* load mode register to active */
    uint32_t txsr;            /* exit self-refresh delay      */
    uint32_t tras;            /* self-refresh time            */
    uint32_t trc;             /* row cycle                    */
    uint32_t twr;             /* write recovery               */
    uint32_t trp;             /* row precharge                */
    uint32_t trcd;            /* row to column                */

    uint32_t refresh_count;   /* see STM32_SDRAM_REFRESH_COUNT() */
    uint32_t powerup_delay_loops;  /* busy-loop count covering >= 100 us */
} stm32_sdram_config_t;

bool stm32_sdram_init(const stm32_sdram_config_t *cfg);

#endif // STM32_SDRAM_H
