#include <errno.h>
#include <printf.h>
#include <string.h>
#include <dm/uclass.h>


/*
 * Start and end of the uclass driver array, set by the linker script around
 * the .dm_uclasses section.
 */
extern const struct uclass_driver _sdm_uclasses[];
extern const struct uclass_driver _edm_uclasses[];


/*
 * One uclass at run time: its uclass driver and its devices, in the order
 * they were bound. To struct uclass_driver what struct udevice is to struct
 * driver. A fixed array for now; a linked list will remove the limit.
 */
struct uclass {
    const struct uclass_driver *uc_drv;  /* NULL until its first device */
    struct udevice *devps[UCLASS_MAX_DEVICES];
    int count;
};

/* One entry per uclass, indexed by enum uclass_id; all empty at boot */
static struct uclass uclasses[UCLASS_COUNT];


/**
 * Linear search by id, as driver_find() searches by name; there are only a
 * handful of uclasses.
 */
const struct uclass_driver *uclass_driver_find(
    enum uclass_id id
)
{
    for (const struct uclass_driver *uc_drv = _sdm_uclasses;
         uc_drv < _edm_uclasses;
         ++uc_drv
        )
    {
        if (uc_drv->id == id)
            return uc_drv;
    }

    return NULL;
}


/**
 * The uclass comes from the device's driver. The ID is checked before it is
 * used as an array index. The uclass driver is looked up once, for the
 * first device, and before the full check, so a missing one is the error
 * reported. Nothing is allocated here, so a failure leaves nothing behind.
 */
int uclass_add_device(
    struct udevice *dev
)
{
    enum uclass_id id = dev->driver->id;

    if (id == UCLASS_INVALID || id >= UCLASS_COUNT)
        return -EINVAL;

    /* First device of this uclass: find its uclass driver */
    if (!uclasses[id].uc_drv)
    {
        uclasses[id].uc_drv = uclass_driver_find(id);

        if (!uclasses[id].uc_drv)
            return -EPFNOSUPPORT;
    }

    int count = uclasses[id].count;

    if (count >= UCLASS_MAX_DEVICES)
        return -ENOMEM;

    uclasses[id].devps[count] = dev;
    uclasses[id].count++;

    return SUCCESS;
}


/**
 * The index is checked against the number of devices bound, not the size of
 * the array, so an unused slot is never returned.
 */
int uclass_find_device(
    enum uclass_id id,
    int index,
    struct udevice **devp
)
{
    if (id == UCLASS_INVALID || id >= UCLASS_COUNT)
        return -EINVAL;

    if (index < 0 || index >= uclasses[id].count)
        return -ENODEV;

    *devp = uclasses[id].devps[index];

    return SUCCESS;
}


/**
 * Starts after UCLASS_INVALID, which never holds a device.
 */
void uclass_print_devices(void)
{
    for (int i = UCLASS_INVALID + 1; i < UCLASS_COUNT; ++i)
    {
        int dev_count = uclasses[i].count;
        /* Not uc_drv: a uclass with no devices never looked its row up */
        const struct uclass_driver *uc_drv = uclass_driver_find(i);

        printf("UClass: %s (%d) - Count: %d\n",
            uc_drv ? uc_drv->name : "(none)",
            i,
            dev_count
        );

        for (int j = 0; j < dev_count; ++j)
        {
            struct udevice *dev = uclasses[i].devps[j];

            /* "-": top level; "(missing)": an orphan, its parent not found */
            printf(" [%d] Name: \"%s\". Parent: %s. Probed: %d\n",
                j,
                dev->name,
                dev->parent ? dev->parent->name :
                    dev->orphan ? "(missing)" : "-",
                dev->probed
            );

            printf(" Drv: %s. Plat: %p. Priv: %p.\n",
                dev->driver->name,
                dev->plat,
                dev->priv
            );

            printf(" UcPriv: %p\n", dev->uclass_priv);
        }
    }
}


/**
 * The device is handed out only after its probe succeeded, so the lookup
 * goes into a local variable first.
 */
int uclass_get_device(
    enum uclass_id id,
    int index,
    struct udevice **devp
)
{
    struct udevice *dev;
    
    int ret = uclass_find_device(id, index, &dev);

    if (ret)
        return ret;

    ret = device_probe(dev);

    if (ret)
        return ret;

    *devp = dev;

    return SUCCESS;
}


/**
 * Only the devices of one uclass are searched, so a name has to be unique
 * within its uclass, not across the system. The loop stops at the number of
 * devices bound: the slots after that are NULL.
 */
int uclass_find_device_by_name(
    enum uclass_id id,
    const char *name,
    struct udevice **devp
)
{
    if (id == UCLASS_INVALID || id >= UCLASS_COUNT)
        return -EINVAL;

    for (int i = 0; i < uclasses[id].count; ++i)
    {
        struct udevice *dev = uclasses[id].devps[i];

        if ( !strcmp(dev->name, name) )
        {
            *devp = dev;
            return SUCCESS;
        }
    }

    return -ENODEV;
}


/**
 * Same shape as uclass_get_device(): look up into a local variable, probe,
 * and hand the device out only if both worked.
 */
int uclass_get_device_by_name(
    enum uclass_id id,
    const char *name,
    struct udevice **devp
)
{
    struct udevice *dev;

    int ret = uclass_find_device_by_name(id, name, &dev);

    if (ret)
        return ret;

    ret = device_probe(dev);

    if (ret)
        return ret;

    *devp = dev;

    return SUCCESS;
}


/**
 * Every uclass in turn, through the per-uclass lookup. A find, never a
 * get: binding must not probe anything.
 */
int device_find_by_name(
    const char *name,
    struct udevice **devp
)
{
    for (int id = UCLASS_INVALID + 1; id < UCLASS_COUNT; ++id)
    {
        if (!uclass_find_device_by_name(id, name, devp))
            return SUCCESS;
    }

    return -ENODEV;
}


/**
 * The same walk as device_find_by_name(). A device from a device_info has
 * DEV_NODE_NONE, which no node offset equals.
 */
int device_find_by_node(
    int node,
    struct udevice **devp
)
{
    for (int id = UCLASS_INVALID + 1; id < UCLASS_COUNT; ++id)
    {
        for (int i = 0; i < uclasses[id].count; ++i)
        {
            struct udevice *dev = uclasses[id].devps[i];

            if (dev->node == node)
            {
                *devp = dev;
                return SUCCESS;
            }
        }
    }

    return -ENODEV;
}


/**
 * Every uclass's count; UCLASS_INVALID never holds a device.
 */
int uclass_device_count(void)
{
    int total = 0;

    for (int id = UCLASS_INVALID + 1; id < UCLASS_COUNT; ++id)
        total += uclasses[id].count;

    return total;
}


/**
 * No id check: the bind checked it, and a bound device's uclass has its
 * uclass driver (set by its first device).
 */
const struct uclass_driver *device_get_uclass_driver(
    const struct udevice *dev
)
{
    return uclasses[dev->driver->id].uc_drv;
}
