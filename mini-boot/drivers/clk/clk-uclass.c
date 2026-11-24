#include <clk.h>
#include <errno.h>
#include <dm/device.h>
#include <dm/read.h>
#include <dm/uclass.h>


/*
 * The clock uclass: the generic half of every clock provider. Consumers
 * (other drivers) get a handle by provider name and clock id, and the
 * calls are dispatched through the provider driver's table of operations,
 * checking both the table and the entry first.
 */

UCLASS_DRIVER(clk) = {
    .name = "clk",
    .id   = UCLASS_CLK,
};


/**
 * Looking the provider up by name with the "get" variant is what probes
 * it: a consumer's probe causes its provider's probe, on first use.
 */
int clk_get_by_name (
    const char *provider,
    u32 id,
    struct clk *clk
)
{
    struct udevice *dev;

    int ret = uclass_get_device_by_name(UCLASS_CLK, provider, &dev);
    if (ret)
        return ret;

    clk->dev = dev;
    clk->id = id;

    return SUCCESS;
}


/**
 * Dispatch to the provider's enable operation.
 */
int clk_enable (
    struct clk *clk
)
{
    const struct clk_ops *ops = clk->dev->driver->ops;

    if (!ops || !ops->enable)
        return -ENOSYS;

    return ops->enable(clk);
}


/**
 * Dispatch to the provider's get_rate operation; 0 if it has none.
 */
u32 clk_get_rate(
    struct clk *clk
)
{
    const struct clk_ops *ops = clk->dev->driver->ops;

    if (!ops || !ops->get_rate)
        return 0;

    return ops->get_rate(clk);
}


/**
 * The phandle names a node; the device bound from that node is the
 * provider. Its name is what clk_get_by_name() looks up later, so the
 * plat contract of every consumer stays the same as with a C table.
 */
int clk_read_by_index(
    const struct udevice *dev,
    int index,
    const char **provider,
    u32 *id
)
{
    struct fdt_phandle_args args;
    struct udevice *prov;

    int ret = dev_read_phandle_with_args(dev, "clocks", "#clock-cells",
                                         index, &args);
    if (ret)
        return ret;

    if (args.count > 1)
        return -EINVAL;

    ret = device_find_by_node(args.node, &prov);
    if (ret)
        return ret;

    *provider = prov->name;
    *id = args.count ? args.args[0] : 0;

    return SUCCESS;
}
