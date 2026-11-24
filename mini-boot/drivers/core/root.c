#include <errno.h>
#include <fdt.h>
#include <printf.h>
#include <dm/device.h>
#include <dm/read.h>
#include <dm/root.h>
#include <dm/uclass.h>


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
    if (!dm_fdt())
        return SUCCESS;

    dm_scan_nodes(dev, dev->node);

    return SUCCESS;
}


/**
 * The nodes under the root, and below them the children every bus binds
 * in its post_bind: the whole tree, in its own order. One bad node must
 * not stop the rest, so a failure is reported and the scan carries on.
 */
int dm_init(void)
{
    if (dm_fdt())
        dm_scan_nodes(NULL, FDT_ROOT);
    else
        printf("Error - No device tree\n");

    return uclass_device_count();
}
