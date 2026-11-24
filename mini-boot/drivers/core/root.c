#include <errno.h>
#include <fdt.h>
#include <printf.h>
#include <dm/device.h>
#include <dm/read.h>
#include <dm/root.h>
#include <dm/uclass.h>


/*
 * Start and end of the board's device table, set by the linker script
 * around the .dm_devices section.
 */
extern const struct device_info _sdm_devices[];
extern const struct device_info _edm_devices[];


/**
 * Bind each child node of 'parent_node' under 'parent'. A disabled node or
 * one without a driver is skipped quietly (-ENODEV: a fixed-clock has
 * none, and needs none); any other failure is reported and the loop
 * carries on.
 */
static void dm_scan_nodes(
    struct udevice *parent,
    int parent_node
)
{
    const void *fdt = dm_fdt();

    for (int node = fdt_first_child(fdt, parent_node);
         node >= 0;
         node = fdt_next_sibling(fdt, node))
    {
        int ret = device_bind_node(parent, node, NULL);

        if (ret && ret != -ENODEV)
            printf("Error %d - Failed to bind node %s\n",
                   ret, fdt_get_name(fdt, node));
    }
}


/**
 * A post_bind hook as it is: a bus's uclass sets .post_bind to this.
 */
int dm_scan_fdt_dev(
    struct udevice *dev
)
{
    if (dev->node == DEV_NODE_NONE || !dm_fdt())
        return SUCCESS;

    dm_scan_nodes(dev, dev->node);

    return SUCCESS;
}


/**
 * The device tree first: the nodes under the root, and below them the
 * children every bus binds in its post_bind. Then the C table, for a
 * device the tree does not describe; its rows can name a device from the
 * tree as their parent.
 *
 * One bad entry must not stop the rest, so a failure is reported and the
 * loop carries on.
 *
 * Two passes for the rows, because the linker does not keep them in any
 * order: a child's row may come before its parent's. Pass 1 binds every
 * row; pass 2, when every device exists, links each child to its parent.
 * So the order never matters, at any depth.
 */
int dm_init(void)
{
    if (dm_fdt())
        dm_scan_nodes(NULL, FDT_ROOT);
    else
        printf("Error - No device tree\n");

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

    return uclass_device_count();
}
