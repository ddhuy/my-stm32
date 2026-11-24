#include <errno.h>
#include <fdt.h>
#include <malloc.h>
#include <string.h>
#include <dm/device.h>
#include <dm/read.h>
#include <dm/uclass.h>


/**
 * "okay" (or "ok", the older spelling) or no status at all: enabled.
 */
static bool node_enabled(
    const void *fdt,
    int node
)
{
    const char *status = fdt_getprop(fdt, node, "status", NULL);

    return !status || !strcmp(status, "okay") || !strcmp(status, "ok");
}


/**
 * The compatible property is a list of strings, most specific first; the
 * first one that has a driver decides.
 */
static const struct driver *node_match(
    const void *fdt,
    int node,
    uintptr_t *datap
)
{
    int len;
    const char *list = fdt_getprop(fdt, node, "compatible", &len);

    if (!list)
        return NULL;

    for (int i = 0; i < len; i += strlen(&list[i]) + 1)
    {
        const struct driver *drv = driver_find_by_compatible(&list[i],
                                                             datap);

        if (drv)
            return drv;
    }

    return NULL;
}


/**
 * Nothing is allocated for a node that is disabled or has no driver: the
 * checks come first. The device joins its uclass before the hooks run, so
 * that its post_bind (a bus binding its children) finds it complete; the
 * parent's child_post_bind runs last, when the child is fully bound. On a
 * failure after the uclass took the device, it stays there: the heap
 * cannot free, and the uclass has no remove.
 */
int device_bind_node(
    struct udevice *parent,
    int node,
    struct udevice **devp
)
{
    const void *fdt = dm_fdt();

    if (!fdt || !node_enabled(fdt, node))
        return -ENODEV;

    uintptr_t data = 0;
    const struct driver *driver = node_match(fdt, node, &data);

    if (!driver)
        return -ENODEV;

    const char *name = fdt_get_label(fdt, node);

    if (!name)
        name = fdt_get_name(fdt, node);

    struct udevice *dev = calloc(1, sizeof(struct udevice));

    if (!dev)
        return -ENOMEM;

    dev->name = name;
    dev->driver = driver;
    dev->node = node;
    dev->driver_data = data;
    dev->parent = parent;

    /* of_to_plat fills it at probe */
    if (driver->plat_auto > 0)
    {
        dev->plat = calloc(1, driver->plat_auto);

        if (!dev->plat)
            return -ENOMEM;
    }

    /* Under a parent: its uclass fills it in child_post_bind */
    const struct uclass_driver *parent_uc_drv =
        parent ? device_get_uclass_driver(parent) : NULL;

    if (parent_uc_drv && parent_uc_drv->per_child_plat_auto > 0)
    {
        dev->parent_plat = calloc(1, parent_uc_drv->per_child_plat_auto);

        if (!dev->parent_plat)
            return -ENOMEM;
    }

    int ret = uclass_add_device(dev);

    /* The device is lost on this path: the heap cannot free */
    if (ret)
        return ret;

    /* Never NULL: uclass_add_device() checked it */
    const struct uclass_driver *uc_drv = device_get_uclass_driver(dev);

    if (uc_drv->post_bind)
    {
        ret = uc_drv->post_bind(dev);

        if (ret)
            return ret;
    }

    if (parent_uc_drv && parent_uc_drv->child_post_bind)
    {
        ret = parent_uc_drv->child_post_bind(dev);

        if (ret)
            return ret;
    }

    if (devp)
        *devp = dev;

    return SUCCESS;
}


/**
 * The parent is probed first, so the recursion goes up one generation per
 * call; it ends at the top: a parent is the node a device is nested in,
 * so there is no cycle.
 * The flag is set last, so a device whose probe failed is never marked
 * ready, not even when only post_probe failed. Its private, uclass and
 * parent data are kept for the retry: the heap cannot free, and allocating
 * again would leak a block per attempt. The hooks run while 'probed' is still
 * false, so a hook must use 'dev' and never look its own device up.
 */
int device_probe(
    struct udevice *dev
)
{
    if (!dev)
        return -EINVAL;

    if (dev->probed)
        return SUCCESS;

    /*
     * Its node into its plat, first: the parent's probe does not need it,
     * and a node that cannot be read costs the parent nothing. Run again
     * on a retry; reading the blob twice gives the same plat.
     */
    if (dev->driver->of_to_plat)
    {
        int ret = dev->driver->of_to_plat(dev);

        if (ret)
            return ret;
    }

    /*
     * The parent first, whatever the driver: a child can only be reached
     * through its parent. Before priv, so a failed parent costs no heap.
     */
    if (dev->parent)
    {
        int ret = device_probe(dev->parent);

        if (ret)
            return ret;
    }

    if (!dev->priv && dev->driver->priv_auto > 0)
    {
        dev->priv = calloc(1, dev->driver->priv_auto);

        if (!dev->priv)
            return -ENOMEM;
    }

    /* Never NULL: a device whose uclass has no uclass driver is not bound */
    const struct uclass_driver *uc_drv = device_get_uclass_driver(dev);

    if (!dev->uclass_priv && uc_drv->per_device_auto > 0)
    {
        dev->uclass_priv = calloc(1, uc_drv->per_device_auto);

        if (!dev->uclass_priv)
            return -ENOMEM;
    }

    /* The parent's uclass, not this one's: it owns the per-child data */
    const struct uclass_driver *parent_uc_drv =
        dev->parent ? device_get_uclass_driver(dev->parent) : NULL;

    if (parent_uc_drv && !dev->parent_priv &&
        parent_uc_drv->per_child_auto > 0)
    {
        dev->parent_priv = calloc(1, parent_uc_drv->per_child_auto);

        if (!dev->parent_priv)
            return -ENOMEM;
    }

    /* The uclass first: it may refuse the driver before any hardware */
    if (uc_drv->pre_probe)
    {
        int ret = uc_drv->pre_probe(dev);

        if (ret)
            return ret;
    }

    /* Then the parent's side: it sets up how this child is reached */
    if (parent_uc_drv && parent_uc_drv->child_pre_probe)
    {
        int ret = parent_uc_drv->child_pre_probe(dev);

        if (ret)
            return ret;
    }

    if (dev->driver->probe)
    {
        int ret = dev->driver->probe(dev);

        if (ret)
            return ret;
    }

    /* The hardware works: what every device of the uclass needs now */
    if (uc_drv->post_probe)
    {
        int ret = uc_drv->post_probe(dev);

        if (ret)
            return ret;
    }

    dev->probed = true;

    return SUCCESS;
}
