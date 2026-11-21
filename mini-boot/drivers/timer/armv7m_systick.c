#include <asm/io.h>
#include <clk.h>
#include <errno.h>
#include <timer.h>
#include <dm/device.h>
#include <dm/driver.h>
#include <platdata/armv7m_systick.h>


/*
 * SysTick: the 24-bit down-counter every ARMv7-M core has. It counts from
 * LOAD to 0, reloads, and (with TICKINT) raises an exception at each wrap.
 * Register layout (ARMv7-M ARM, B3.3), offsets from plat->base.
 */
#define SYSTICK_CTRL  0x0
#define SYSTICK_LOAD  0x4
#define SYSTICK_VAL   0x8

#define SYSTICK_TICKS_PER_SEC  1000
#define SYSTICK_LOAD_MAX       0x00FFFFFF

#define SYSTICK_CTRL_ENABLE     BIT(0)
#define SYSTICK_CTRL_TICKINT    BIT(1)
#define SYSTICK_CTRL_CLKSOURCE  BIT(2)


/*
 * Called only from the vector table (slot 15 in start.S), so no header
 * declares it. The prototype is here for -Wmissing-prototypes. The name
 * must match start.S exactly: a different one (CMSIS SysTick_Handler)
 * builds cleanly but leaves the weak default_handler in the table.
 */
void systick_handler(void);


/**
 * The device the interrupt counts for. The hardware calls the handler with
 * no arguments, so it cannot be given a device; this pointer is the only
 * way back. One SysTick per CPU makes one pointer enough, and probe refuses
 * a second device rather than take the interrupt away from the first.
 */
static struct udevice *systick_dev = NULL;


/**
 * Per-device data the driver changes at run time, so it lives on the heap
 * (priv_auto) rather than in the constant platform data.
 */
struct armv7m_systick_priv
{
    u32 rate;            /* input clock, Hz */
    volatile u32 ticks;  /* incremented by the interrupt */
};


/**
 * Start the counter at SYSTICK_TICKS_PER_SEC wraps per second, with an
 * interrupt at each wrap. The core has already allocated priv (zeroed, so
 * ticks starts at 0).
 *
 * Returns 0; -EBUSY if another device owns the interrupt; the clock
 * provider's error (-ENODEV: no provider of that name); -EINVAL if the
 * rate is 0 or too high for the 24-bit reload.
 */
static int armv7m_systick_probe (
    struct udevice *dev
)
{
    /* Checked before any register is touched, so a refusal changes nothing */
    if (systick_dev)
        return -EBUSY;

    const struct armv7m_systick_plat *plat = dev->plat;
    struct clk clk;
    int ret;

    ret = clk_get_by_name(plat->clk_dev, plat->clk_id, &clk);
    if (ret)
        return ret;

    ret = clk_enable(&clk);
    if (ret)
        return ret;

    /* The rate comes from the provider, never a hardcoded value */
    u32 rate = clk_get_rate(&clk);
    if (!rate)
        return -EINVAL;

    /* The counter wraps after reload + 1 cycles; reload must fit 24 bits */
    u32 reload = rate / SYSTICK_TICKS_PER_SEC - 1;
    if (reload > SYSTICK_LOAD_MAX)
        return -EINVAL;

    /* Remember the input clock the reload was computed from */
    struct armv7m_systick_priv *priv = dev->priv;
    priv->rate = rate;

    /*
     * Everything the handler uses must be ready before TICKINT is set: from
     * then on it can run at any moment. So it needs no NULL check.
     */
    systick_dev = dev;

    /*
     * LOAD first, then clear VAL (any write does) so the first period is a
     * full one, then CTRL last to start. CLKSOURCE = 1: count the processor
     * clock, which plat->clk_id names (see platdata/armv7m_systick.h).
     * TICKINT = 1: SysTick is a system exception, so no NVIC enable is
     * needed.
     */
    writel(reload, plat->base + SYSTICK_LOAD);
    writel(0, plat->base + SYSTICK_VAL);
    writel(SYSTICK_CTRL_CLKSOURCE | SYSTICK_CTRL_TICKINT | SYSTICK_CTRL_ENABLE,
        plat->base + SYSTICK_CTRL);

    return SUCCESS;
}


/**
 * One tick per wrap, every millisecond. Kept to one increment: anything slow
 * here (a printf) would take longer than the time between two ticks.
 */
void systick_handler(void)
{
    struct armv7m_systick_priv *priv = systick_dev->priv;
    priv->ticks++;
}


/**
 * Milliseconds since probe: one tick is one millisecond. ticks is volatile
 * and one aligned 32-bit word, so this read is never torn by the handler.
 */
static u32 armv7m_systick_get_ms (
    struct udevice *dev
)
{
    struct armv7m_systick_priv *priv = dev->priv;

    return priv->ticks;
}


/**
 * The timer operations this driver provides.
 */
static const struct timer_ops armv7m_systick_ops = {
    .get_ms = armv7m_systick_get_ms
};


/**
 * Generic for any ARMv7-M core, so the name carries no SoC prefix.
 */
DM_DRIVER(armv7m_systick) = {
    .name = "armv7m_systick",
    .id   = UCLASS_TIMER,

    .probe = armv7m_systick_probe,

    .priv_auto = sizeof(struct armv7m_systick_priv),

    .ops = &armv7m_systick_ops,
};
