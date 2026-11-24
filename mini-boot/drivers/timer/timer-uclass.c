#include <timer.h>
#include <dm/device.h>
#include <dm/uclass.h>


/*
 * The timer uclass: the generic half of every timer driver. It finds the
 * driver's table of operations through the device and calls the matching
 * entry, checking both the table and the entry first.
 */

UCLASS_DRIVER(timer) = {
    .name = "timer",
    .id   = UCLASS_TIMER,
};


/**
 * A time has no room for an error code, so a driver that does not provide
 * the operation reads as 0.
 */
u32 timer_get_ms (
    struct udevice *dev
)
{
    const struct timer_ops *ops = dev->driver->ops;

    if (!ops || !ops->get_ms)
        return 0;

    return ops->get_ms(dev);
}
