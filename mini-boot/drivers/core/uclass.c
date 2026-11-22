#include <errno.h>
#include <printf.h>
#include <string.h>
#include <dm/uclass.h>


/*
 * The devices of one uclass, in the order they were bound. A fixed array
 * for now; a linked list will remove the limit.
 */
struct uclass_devices {
    struct udevice *devps[UCLASS_MAX_DEVICES];
    int count;
};

/* One entry per uclass, indexed by enum uclass_id; all empty at boot */
static struct uclass_devices uc_devices[UCLASS_COUNT];


/**
 * The uclass comes from the device's driver. The ID is checked before it is
 * used as an array index.
 */
int uclass_add_device(
    struct udevice *dev
)
{
    enum uclass_id id = dev->driver->id;

    if (id == UCLASS_INVALID || id >= UCLASS_COUNT)
        return -EINVAL;

    int count = uc_devices[id].count;

    if (count >= UCLASS_MAX_DEVICES)
        return -ENOMEM;

    uc_devices[id].devps[count] = dev;
    uc_devices[id].count++;

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

    if (index < 0 || index >= uc_devices[id].count)
        return -ENODEV;

    *devp = uc_devices[id].devps[index];

    return SUCCESS;
}


/**
 * Starts after UCLASS_INVALID, which never holds a device.
 */
void uclass_print_devices(void)
{
    for (int i = UCLASS_INVALID + 1; i < UCLASS_COUNT; ++i)
    {
        int dev_count = uc_devices[i].count;

        printf("UClass-Id: %d - Count: %d\n", i, dev_count);

        for (int j = 0; j < dev_count; ++j)
        {
            struct udevice *dev = uc_devices[i].devps[j];

            /* "-": top level; "(missing)": an orphan, its parent not found */
            printf("  [%d] Name: \"%s\". Parent: %s. Probed: %d\n",
                j,
                dev->name,
                dev->parent ? dev->parent->name :
                    dev->orphan ? "(missing)" : "-",
                dev->probed
            );

            printf("      Drv: \"%s\". Plat: %p. Priv: %p\n",
                dev->driver->name,
                dev->plat,
                dev->priv
            );
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

    for (int i = 0; i < uc_devices[id].count; ++i)
    {
        struct udevice *dev = uc_devices[id].devps[i];

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
