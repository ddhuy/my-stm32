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

    int ret = uclass_add_device(dev);

    /* The device is lost on this path: the heap cannot free */
    if (ret)
        return ret;

    *devp = dev;

    return SUCCESS;
}


/**
 * 
 */
int device_probe(
    struct udevice *dev
)
{
    if (!dev)
        return -EINVAL;
    
    if (dev->probed)
        return SUCCESS;

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
