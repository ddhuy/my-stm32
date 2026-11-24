#include <errno.h>
#include <malloc.h>
#include <dm/device.h>
#include <dm/uclass.h>


/**
 * The driver is looked up before anything is allocated, so a device without
 * a driver costs no heap.
 */
int device_bind(
    const struct device_info *info,
    struct udevice **devp
)
{
    const struct driver *driver = driver_find(info->driver);

    if (!driver)
        return -ENOENT;

    struct udevice *dev = calloc(1, sizeof(struct udevice));

    if (!dev)
        return -ENOMEM;

    dev->name = info->name;
    dev->driver = driver;
    dev->plat = info->plat;
    /* Waiting for device_set_parent(); refused by probe until then */
    dev->orphan = (info->parent != NULL);

    int ret = uclass_add_device(dev);

    /* The device is lost on this path: the heap cannot free */
    if (ret)
        return ret;

    *devp = dev;

    return SUCCESS;
}


/**
 * Only the link that closes a cycle can create one, and at that moment
 * the rest of it is already linked, so walking up from the parent finds
 * 'dev'. The walk ends: there is no cycle before this link.
 */
int device_set_parent(
    struct udevice *dev,
    struct udevice *parent,
    const void *parent_plat
)
{
    for (const struct udevice *up = parent; up; up = up->parent)
        if (up == dev)
            return -EINVAL;

    dev->parent = parent;
    dev->parent_plat = parent_plat;
    dev->orphan = false;

    return SUCCESS;
}


/**
 * The parent is probed first, so the recursion goes up one generation per
 * call (device_set_parent() refuses a cycle, which would never end).
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

    /* Its parent is missing or was refused: it cannot be reached */
    if (dev->orphan)
        return -ENODEV;

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
