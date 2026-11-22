#include <errno.h>
#include <printf.h>
#include <dm/device.h>
#include <dm/root.h>
#include <dm/uclass.h>


/*
 * Start and end of the board's device table, set by the linker script
 * around the .dm_devices section.
 */
extern const struct device_info _sdm_devices[];
extern const struct device_info _edm_devices[];


/**
 * One bad entry must not stop the rest, so a failure is reported and the
 * loop carries on.
 *
 * Two passes, because the linker does not keep the rows in any order: a
 * child's row may come before its parent's. Pass 1 binds every row; pass
 * 2, when every device exists, links each child to its parent. So the
 * order never matters, at any depth.
 */
int dm_init(void)
{
    int dev_count = 0;

    /* Pass 1: bind every row, with no parent yet */
    for (const struct device_info *dev_info = _sdm_devices;
        dev_info < _edm_devices;
        ++dev_info
    )
    {
        struct udevice *dev;

        int ret = device_bind(dev_info, &dev);

        if (ret)
        {
            printf("Error %d - Failed to bind. Device %s. Driver %s\n",
                ret,
                dev_info->name,
                dev_info->driver
            );
            continue;
        }

        dev_count++;
    }

    /*
     * Pass 2: link children to parents. A child whose parent is missing or
     * refused stays bound as an orphan, which device_probe() refuses
     * (-ENODEV); it still shows in the device list.
     */
    for (const struct device_info *dev_info = _sdm_devices;
        dev_info < _edm_devices;
        ++dev_info
    )
    {
        int ret;
        struct udevice *dev, *parent;

        /* Device has no parent */
        if (!dev_info->parent)
            continue;

        /* Not found: it failed to bind in pass 1, already reported */
        ret = device_find_by_name(dev_info->name, &dev);
        if (ret)
            continue;

        ret = device_find_by_name(dev_info->parent, &parent);
        if (ret)
        {
            printf("Error %d - No parent %s for device %s\n",
                    ret, dev_info->parent, dev_info->name);
            continue;
        }

        /* -EINVAL: the link would make a cycle */
        ret = device_set_parent(dev, parent, dev_info->parent_plat);
        if (ret)
            printf("Error %d - Parent %s of device %s is a cycle\n",
                    ret, dev_info->parent, dev_info->name);
    }

    return dev_count;
}
