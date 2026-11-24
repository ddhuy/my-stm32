#include <asm/io.h>
#include <clk.h>
#include <errno.h>
#include <dm/device.h>
#include <dm/driver.h>
#include <platdata/lm3s_sysctl.h>
#include <fdt.h>
#include <dm/read.h>


/*
 * TI Stellaris LM3S System Control: the clock provider. Probe runs the
 * system clock from the PLL; the operations gate peripheral clocks and
 * report the rate. Register layout from the LM3S6965 datasheet, offsets
 * from plat->base. The gate registers RCGC0..2 are consecutive, in the
 * order of the LM3S_CLK_RCGC* numbers in platdata/lm3s_sysctl.h.
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

/* An id is (gate register * 32 + bit); see LM3S_CLK_ID() */
#define CLK_ID_REG(id)      ((id) / 32)
#define CLK_ID_BIT(id)      ((id) % 32)

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
 * Run the system clock from the PLL at plat->sysclk_hz, with a crystal of
 * plat->xtal_hz on the main oscillator. The order is the datasheet's (and
 * TI driverlib's SysCtlClockSet()): bypass, set up and power the PLL, set
 * the divider, wait for lock, then switch over.
 *
 * Never calls the clk_* API: that would look this device up and probe it
 * again, before 'probed' is set, and recurse until the stack overflows.
 *
 * Returns 0; -EINVAL if the crystal is not in the XTAL table or PLL_HZ /
 * sysclk_hz is not a whole divider from SYSDIV_MIN + 1 to SYSDIV_MAX + 1;
 * -ETIMEDOUT if the PLL does not lock (the CPU then stays on the raw
 * oscillator).
 */
static int lm3s_sysctl_probe(
    struct udevice *dev
)
{
    const struct lm3s_sysctl_plat *plat = dev->plat;
    u32 xtal;

    for (xtal = 0; xtal < ARRAY_SIZE(xtal_hz_table); xtal++)
        if (xtal_hz_table[xtal] && xtal_hz_table[xtal] == plat->xtal_hz)
            break;
    if (xtal == ARRAY_SIZE(xtal_hz_table))
        return -EINVAL;

    /* Reject rather than round: 30 MHz would otherwise come out as 33 */
    if (plat->sysclk_hz == 0 || PLL_HZ % plat->sysclk_hz)
        return -EINVAL;

    u32 sysdiv = PLL_HZ / plat->sysclk_hz - 1;
    if (sysdiv < SYSDIV_MIN || sysdiv > SYSDIV_MAX)
        return -EINVAL;

    uintptr_t rcc = plat->base + RCC;
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
    writel(RIS_PLLLRIS, plat->base + MISC);
    writel(val, rcc);

    /* 3. The divider, still bypassed */
    val &= ~RCC_SYSDIV_MASK;
    val |= (sysdiv << RCC_SYSDIV_SHIFT) | RCC_USESYSDIV;
    writel(val, rcc);

    /* 4. Wait for lock; RIS, not MISC (MISC is masked by IMC, which is 0) */
    u32 loops = PLL_LOCK_LOOPS;
    while (!(readl(plat->base + RIS) & RIS_PLLLRIS))
        if (--loops == 0)
            return -ETIMEDOUT;

    /* 5. Switch the system clock over to the PLL */
    val &= ~RCC_BYPASS;
    writel(val, rcc);

    return SUCCESS;
}


/**
 * Turn on one peripheral clock: set its bit in RCGC0..2. Core clocks
 * (LM3S_CLK_BUS_CORE) have no gate, so there is nothing to do. Any other
 * register number is refused: it would write to whatever follows RCGC2.
 */
static int lm3s_sysctl_enable(
    struct clk *clk
)
{
    const struct lm3s_sysctl_plat *plat = clk->dev->plat;
    u32 reg = CLK_ID_REG(clk->id);

    if (reg == LM3S_CLK_BUS_CORE)
        return SUCCESS;
    if (reg > LM3S_CLK_RCGC2)
        return -EINVAL;

    uintptr_t rcgc = plat->base + RCGC(reg);

    setbits(rcgc, BIT(CLK_ID_BIT(clk->id)));
    /* the clock needs 3 cycles before the peripheral can be accessed */
    (void)readl(rcgc);

    return SUCCESS;
}


/**
 * Every peripheral here runs on SysClk, so the id does not matter.
 * Computed from RCC, for the one configuration where the real chip and
 * QEMU agree: PLL on, not bypassed, through SYSDIV. Anything else (the
 * reset state, RCC2 in use) returns 0, which consumers reject.
 */
static u32 lm3s_sysctl_get_rate(
    struct clk *clk
)
{
    const struct lm3s_sysctl_plat *plat = clk->dev->plat;

    if (readl(plat->base + RCC2) & RCC2_USERCC2)
        return 0;

    u32 rcc = readl(plat->base + RCC);

    if ((rcc & RCC_BYPASS) || !(rcc & RCC_USESYSDIV))
        return 0;

    return PLL_HZ / (((rcc & RCC_SYSDIV_MASK) >> RCC_SYSDIV_SHIFT) + 1);
}


/**
 * This driver's implementation of the clock uclass operations.
 */
static const struct clk_ops lm3s_sysctl_ops = {
    .enable   = &lm3s_sysctl_enable,
    .get_rate = &lm3s_sysctl_get_rate,
};


/**
 * System Control's node into its plat: base, the crystal from the node
 * that clocks = <&clk_xtal> points to (a fixed-clock, no device: its
 * clock-frequency is read directly), and the system clock to run from
 * clock-frequency.
 *
 * Returns 0 or the error from reading the nodes.
 */
static int lm3s_sysctl_of_to_plat(
    struct udevice *dev
)
{
    struct lm3s_sysctl_plat *plat = dev->plat;
    struct fdt_phandle_args xtal;
    int ret;

    ret = dev_read_addr_index(dev, 0, &plat->base);
    if (ret)
        return ret;

    ret = dev_read_phandle_with_args(dev, "clocks", "#clock-cells", 0,
                                     &xtal);
    if (ret)
        return ret;

    ret = fdt_read_u32(dm_fdt(), xtal.node, "clock-frequency",
                       &plat->xtal_hz);
    if (ret)
        return ret;

    return dev_read_u32(dev, "clock-frequency", &plat->sysclk_hz);
}


/**
 * The compatible strings this driver handles.
 */
static const struct udevice_id lm3s_sysctl_ids[] = {
    { .compatible = "ti,lm3s6965-sysctl" },
    { }
};

/**
 * Driver model entry. No private data: the rate is read back from RCC.
 */
DM_DRIVER(lm3s_sysctl) = {
    .name = "lm3s_sysctl",
    .id   = UCLASS_CLK,
    .of_match = lm3s_sysctl_ids,

    .of_to_plat = lm3s_sysctl_of_to_plat,
    .plat_auto  = sizeof(struct lm3s_sysctl_plat),

    .probe = &lm3s_sysctl_probe,

    .ops = &lm3s_sysctl_ops,
};
