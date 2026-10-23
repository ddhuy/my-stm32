#include <stdbool.h>
#include <stdint.h>

#include "soc/stm32f429/fmc_regs.h"

#include "drivers/ram/stm32_sdram.h"


#define FMC_TIMEOUT 1000000u

/* JEDEC auto-refresh cycles during initialization (at least 2) */
#define SDRAM_INIT_AUTOREFRESH  8u

/* SDRAM chip mode register (JEDEC), sent with the LOAD MODE command */
#define SDRAM_MODE_BURST_LENGTH_1      (0u << 0)
#define SDRAM_MODE_BURST_SEQUENTIAL    (0u << 3)
#define SDRAM_MODE_CAS_LATENCY(c)      ((c) << 4)
#define SDRAM_MODE_OPERATING_STANDARD  (0u << 7)
#define SDRAM_MODE_WRITEBURST_SINGLE   (1u << 9)


static void spin(uint32_t n)
{
    while (n--) {
        __asm volatile("nop");
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

static bool fmc_cmd(uint32_t cmd, uint32_t target)
{
    if (!fmc_wait())
        return false;

    FMC_SDCMR = cmd | target;

    if (!fmc_wait())
        return false;

    return true;
}

bool stm32_sdram_init(const stm32_sdram_config_t *cfg)
{
    uint32_t shared = cfg->sdclk | FMC_SDCR_RPIPE(cfg->read_pipe) |
                      (cfg->read_burst ? FMC_SDCR_RBURST : 0u);
    uint32_t geometry = FMC_SDCR_NC(cfg->column_bits) | FMC_SDCR_NR(cfg->row_bits) |
                        cfg->bus_width | FMC_SDCR_CAS(cfg->cas_latency) |
                        (cfg->internal_banks == 4u ? FMC_SDCR_NB_4 : 0u);
    uint32_t trc_trp = FMC_SDTR_TRC(cfg->trc) | FMC_SDTR_TRP(cfg->trp);
    uint32_t other_timings = FMC_SDTR_TMRD(cfg->tmrd) | FMC_SDTR_TXSR(cfg->txsr) |
                             FMC_SDTR_TRAS(cfg->tras) | FMC_SDTR_TWR(cfg->twr) |
                             FMC_SDTR_TRCD(cfg->trcd);
    uint32_t target;

    if (cfg->bank == 1u)
    {
        FMC_SDCR1 = shared | geometry;
        FMC_SDTR1 = trc_trp | other_timings;
        target = FMC_CMD_CTB1;
    }
    else if (cfg->bank == 2u)
    {
        /* Bank 2: SDCLK/RBURST/RPIPE live in SDCR1, TRC/TRP in SDTR1. */
        FMC_SDCR1 = shared;
        FMC_SDCR2 = geometry;
        FMC_SDTR1 = trc_trp;
        FMC_SDTR2 = other_timings;
        target = FMC_CMD_CTB2;
    }
    else
    {
        return false;
    }

    if (!fmc_cmd(FMC_CMD_CLK_ENABLE, target))
        return false;

    spin(cfg->powerup_delay_loops);

    if (!fmc_cmd(FMC_CMD_PALL, target))  /* precharge all */
        return false;

    if (!fmc_cmd(FMC_CMD_AUTOREFRESH | FMC_SDCMR_NRFS(SDRAM_INIT_AUTOREFRESH), target))
        return false;

    uint32_t mode = SDRAM_MODE_BURST_LENGTH_1 | SDRAM_MODE_BURST_SEQUENTIAL |
                    SDRAM_MODE_CAS_LATENCY(cfg->cas_latency) |
                    SDRAM_MODE_OPERATING_STANDARD | SDRAM_MODE_WRITEBURST_SINGLE;

    if (!fmc_cmd(FMC_CMD_LOAD_MODE | FMC_SDCMR_MRD(mode), target))
        return false;

    FMC_SDRTR = FMC_SDRTR_COUNT(cfg->refresh_count);

    return fmc_wait();
}
