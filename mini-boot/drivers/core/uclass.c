#include <errno.h>
#include <printf.h>
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
            struct udevice *devp = uc_devices[i].devps[j];
            printf("\t[%d] Name: \"%s\". Drv: \"%s\". Plat: %p.\n",
                j,
                devp->name,
                devp->driver->name,
                devp->plat
            );
        }
    }
}
