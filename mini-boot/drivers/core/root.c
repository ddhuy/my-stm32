#include <printf.h>
#include <dm/device.h>
#include <dm/root.h>


/*
 * Start and end of the board's device table, set by the linker script
 * around the .dm_devices section.
 */
extern const struct device_info _sdm_devices[];
extern const struct device_info _edm_devices[];


/**
 * One bad entry must not stop the rest, so a failure is reported and the
 * loop carries on.
 */
int dm_init(void)
{
    int dev_count = 0;

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

    return dev_count;
}
