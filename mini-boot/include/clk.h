#ifndef MINIBOOT_CLK_H
#define MINIBOOT_CLK_H

#include <types.h>


struct udevice;


/**
 * A handle to one clock: clock 'id' of the provider device 'dev'. What an
 * id means is the provider's business (its platdata header defines them);
 * a consumer only passes it on. Small enough to keep on the stack: every
 * consumer uses its clock only in probe.
 */
struct clk
{
    struct udevice *dev;
    u32 id;
};


/**
 * The operations every clock provider implements. Both take the handle,
 * not the device: the provider needs its device (for the platdata) and
 * the id.
 *
 * enable:   turn the clock on; returns 0 or -Exxx
 * get_rate: frequency in Hz, 0 if unknown
 */
struct clk_ops
{
    int (*enable) (struct clk *clk);
    u32 (*get_rate) (struct clk *clk);
};


/**
 * Get clock 'id' of the clock provider called 'provider', probing the
 * provider if this is its first use. The handle is filled in only on
 * success.
 *
 * Returns 0, or the error from looking up or probing the provider
 * (-ENODEV: no provider of that name).
 */
int clk_get_by_name (
    const char *provider,
    u32 id,
    struct clk *clk
);


/**
 * Turn the clock on.
 *
 * Returns 0, -ENOSYS if the provider has no enable operation, or the
 * provider's error.
 */
int clk_enable (
    struct clk *clk
);


/**
 * Frequency of the clock in Hz. A rate has no room for an error code, so 0
 * means unknown (or no get_rate operation); consumers must reject it.
 */
u32 clk_get_rate(
    struct clk *clk
);


/**
 * For a consumer's of_to_plat: entry 'index' of its node's clocks list
 * (clocks = <&rcc 164>), as the name of the provider's device and the
 * clock id (the one argument cell; 0 for a provider with #clock-cells 0).
 * What the consumer's probe then passes to clk_get_by_name(). The provider
 * is looked up, not probed: binding is over, probing has not started.
 *
 * Returns 0; -ENODEV if the consumer has no node or the phandle points to
 * a node without a device (a fixed-clock: read its clock-frequency
 * instead); -EINVAL if the provider has more than one argument cell; or
 * the error from reading the list.
 */
int clk_read_by_index(
    const struct udevice *dev,
    int index,
    const char **provider,
    u32 *id
);

#endif /* MINIBOOT_CLK_H */
