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
 * ready. Its private data is kept for the retry: the heap cannot free, and
 * allocating again would leak a block per attempt.
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

    if (dev->driver->probe)
    {
        int ret = dev->driver->probe(dev);

        if (ret)
            return ret;
    }

    dev->probed = true;

    return SUCCESS;
}
