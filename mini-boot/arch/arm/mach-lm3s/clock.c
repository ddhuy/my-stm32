#include <asm/io.h>
#include <errno.h>
#include <mach/clock_ids.h>
#include <mach/lm3s6965.h>
#include <mach/sysctl.h>
#include <clock.h>


/*
 * System Control register layout. The clock-gating registers RCGC0..2 are
 * consecutive, in the order of the CLK_RCGC* numbers in mach/clock_ids.h.
 */
#define RIS             0x050
#define MISC            0x058
#define RCC             0x060
#define RCC2            0x070
#define RCGC0           0x100
#define RCGC(reg)       (RCGC0 + 4 * (reg))

#define RIS_PLLLRIS     BIT(6)  /* PLL locked (MISC: write 1 to clear) */

#define RCC_MOSCDIS         BIT(0)  /* main oscillator disable */
#define RCC_OSCSRC_SHIFT    4
#define RCC_OSCSRC_MASK     (0x3 << RCC_OSCSRC_SHIFT)
#define RCC_OSCSRC_MAIN     (0x0 << RCC_OSCSRC_SHIFT)
#define RCC_XTAL_SHIFT      6
#define RCC_XTAL_MASK       (0xF << RCC_XTAL_SHIFT)
#define RCC_BYPASS          BIT(11) /* system clock bypasses the PLL */
#define RCC_OEN             BIT(12) /* PLL output disable (active low) */
#define RCC_PWRDN           BIT(13) /* PLL power down */
#define RCC_USESYSDIV       BIT(22) /* system clock goes through SYSDIV */
#define RCC_SYSDIV_SHIFT    23
#define RCC_SYSDIV_MASK     (0xF << RCC_SYSDIV_SHIFT)

#define RCC2_USERCC2        BIT(31) /* RCC2 overrides RCC */

/* PLL output as SYSDIV sees it: 400 MHz with its fixed /2 */
#define PLL_HZ              200000000

/* SysClk = PLL_HZ / (SYSDIV + 1); 3 gives 50 MHz, the chip's maximum */
#define SYSDIV_MIN          3
#define SYSDIV_MAX          15

/*
 * No timer exists yet, so the lock wait is bounded by a loop count. The
 * real PLL locks in well under a millisecond; this allows far more.
 */
#define PLL_LOCK_LOOPS      100000


/*
 * Crystal frequency for each RCC XTAL value, from the LM3S6965 datasheet.
 * Values 0x0-0x3 are reserved (0 here).
 */
static const u32 xtal_hz_table[] = {
    [0x4] = 3579545,
    [0x5] = 3686400,
    [0x6] = 4000000,
    [0x7] = 4096000,
    [0x8] = 4915200,
    [0x9] = 5000000,
    [0xA] = 5120000,
    [0xB] = 6000000,
    [0xC] = 6144000,
    [0xD] = 7372800,
    [0xE] = 8000000,
    [0xF] = 8192000,
};


/**
 * Run the system clock from the PLL at sysclk_hz, with a crystal of
 * xtal_hz on the main oscillator. The order is the datasheet's (and TI
 * driverlib's SysCtlClockSet()): bypass, set up and power the PLL, set the
 * divider, wait for lock, then switch over.
 *
 * Returns 0; -EINVAL if the crystal is not in the XTAL table or PLL_HZ /
 * sysclk_hz is not a whole divider from SYSDIV_MIN + 1 to SYSDIV_MAX + 1;
 * -ETIMEDOUT if the PLL does not lock (the CPU then stays on the raw
 * oscillator).
 */
int lm3s_clock_init(
    u32 xtal_hz,
    u32 sysclk_hz
)
{
    u32 xtal;

    for (xtal = 0; xtal < ARRAY_SIZE(xtal_hz_table); xtal++)
        if (xtal_hz_table[xtal] && xtal_hz_table[xtal] == xtal_hz)
            break;
    if (xtal == ARRAY_SIZE(xtal_hz_table))
        return -EINVAL;

    /* Reject rather than round: 30 MHz would otherwise come out as 33 */
    if (sysclk_hz == 0 || PLL_HZ % sysclk_hz)
        return -EINVAL;

    u32 sysdiv = PLL_HZ / sysclk_hz - 1;
    if (sysdiv < SYSDIV_MIN || sysdiv > SYSDIV_MAX)
        return -EINVAL;

    uintptr_t rcc = SYSCTL_BASE + RCC;
    u32 val = readl(rcc);

    /* 1. Run from the raw clock while the PLL changes */
    val |= RCC_BYPASS;
    val &= ~RCC_USESYSDIV;
    writel(val, rcc);

    /*
     * 2. Crystal, main oscillator on, PLL powered and its output enabled.
     * Clear an old lock flag first, so that only this power-up counts.
     */
    val &= ~(RCC_XTAL_MASK | RCC_OSCSRC_MASK | RCC_PWRDN | RCC_OEN |
             RCC_MOSCDIS);
    val |= (xtal << RCC_XTAL_SHIFT) | RCC_OSCSRC_MAIN;
    writel(RIS_PLLLRIS, SYSCTL_BASE + MISC);
    writel(val, rcc);

    /* 3. The divider, still bypassed */
    val &= ~RCC_SYSDIV_MASK;
    val |= (sysdiv << RCC_SYSDIV_SHIFT) | RCC_USESYSDIV;
    writel(val, rcc);

    /* 4. Wait for lock; RIS, not MISC (MISC is masked by IMC, which is 0) */
    u32 loops = PLL_LOCK_LOOPS;
    while (!(readl(SYSCTL_BASE + RIS) & RIS_PLLLRIS))
        if (--loops == 0)
            return -ETIMEDOUT;

    /* 5. Switch the system clock over to the PLL */
    val &= ~RCC_BYPASS;
    writel(val, rcc);

    return SUCCESS;
}


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
 * Frequency in Hz of the clock feeding one peripheral. Every peripheral
 * here runs on SysClk. Computed from RCC, for the one configuration where
 * the real chip and QEMU agree: PLL on, not bypassed, through SYSDIV.
 * Anything else (the reset state, RCC2 in use) returns 0, which drivers
 * reject.
 */
u32 clock_get_rate(u32 clk_id)
{
    (void)clk_id;

    if (readl(SYSCTL_BASE + RCC2) & RCC2_USERCC2)
        return 0;

    u32 rcc = readl(SYSCTL_BASE + RCC);

    if ((rcc & RCC_BYPASS) || !(rcc & RCC_USESYSDIV))
        return 0;

    return PLL_HZ / (((rcc & RCC_SYSDIV_MASK) >> RCC_SYSDIV_SHIFT) + 1);
}
