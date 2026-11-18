#include <asm/io.h>
#include <clock.h>
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
 * Start the counter at SYSTICK_TICKS_PER_SEC wraps per second. The core has
 * already allocated priv. The interrupt stays off until there is a handler.
 */
static int armv7m_systick_probe (
    struct udevice *dev
)
{
    const struct armv7m_systick_plat *plat = dev->plat;

    /* The rate comes from the SoC's clock code, never a hardcoded value */
    u32 rate = clock_get_rate(plat->clk_id);
    if (!rate)
        return -EINVAL;

    /* The counter wraps after reload + 1 cycles; reload must fit 24 bits */
    u32 reload = rate / SYSTICK_TICKS_PER_SEC - 1;
    if (reload > SYSTICK_LOAD_MAX)
        return -EINVAL;

    /*
     * LOAD first, then clear VAL (any write does) so the first period is a
     * full one, then CTRL last to start. CLKSOURCE = 1: count the processor
     * clock, which plat->clk_id names (see platdata/armv7m_systick.h).
     */
    writel(reload, plat->base + SYSTICK_LOAD);
    writel(0, plat->base + SYSTICK_VAL);
    writel(SYSTICK_CTRL_CLKSOURCE | SYSTICK_CTRL_ENABLE,
        plat->base + SYSTICK_CTRL);
    
    /* Remember the input clock: it can change at run time once the PLL is in */
    struct armv7m_systick_priv *priv = dev->priv;
    priv->rate = rate;

    return SUCCESS;
}


/**
 * Milliseconds since probe: one tick is one millisecond. Enabled in Part B,
 * when the interrupt starts counting.
 */
// static u32 armv7m_systick_get_ms (
//     struct udevice *dev
// )
// {
//     struct armv7m_systick_priv *priv = dev->priv;

//     return priv->ticks;
// }


/**
 * The timer operations this driver provides.
 */
// static const struct timer_ops armv7m_systick_ops = {
//     .get_ms = armv7m_systick_get_ms
// };


/**
 * Generic for any ARMv7-M core, so the name carries no SoC prefix. Without
 * .ops (until Part B) timer_get_ms() returns 0.
 */
DM_DRIVER(armv7m_systick) = {
    .name = "armv7m_systick",
    .id   = UCLASS_TIMER,

    .probe = armv7m_systick_probe,

    .priv_auto = sizeof(struct armv7m_systick_priv),

    // .ops = &armv7m_systick_ops,
};
