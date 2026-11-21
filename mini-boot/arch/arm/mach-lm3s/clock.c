#include <asm/io.h>
#include <mach/clock_ids.h>
#include <mach/lm3s6965.h>
#include <clock.h>


/*
 * System Control register layout. The clock-gating registers RCGC0..2 are
 * consecutive, in the order of the CLK_RCGC* numbers in mach/clock_ids.h.
 */
#define RCGC0           0x100
#define RCGC(reg)       (RCGC0 + 4 * (reg))

/* QEMU's reset value; Part C computes it from RCC */
#define SYSCLK_HZ       12500000


/**
 * Turn on the clock of one peripheral
 */
void clock_enable(u32 clk_id)
{
    /*
     * Only RCGC0..2 exist. Core clocks (CLK_BUS_CORE) have no gate, and any
     * other number would write to whatever follows RCGC2.
     */
    if (clk_id / 32 > CLK_RCGC2)
        return;

    uintptr_t rcgc = SYSCTL_BASE + RCGC(clk_id / 32);

    setbits(rcgc, BIT(clk_id % 32));
    /* the clock needs 3 cycles before the peripheral can be accessed */
    (void)readl(rcgc);
}


/**
 * Frequency in Hz of the clock feeding one peripheral
 */
u32 clock_get_rate(u32 clk_id)
{
    /* No PLL setup: every peripheral runs on the system clock */
    (void)clk_id;

    return SYSCLK_HZ;
}
