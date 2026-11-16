#ifndef MINIBOOT_DM_UCLASS_H
#define MINIBOOT_DM_UCLASS_H

#include <dm/device.h>
#include <dm/uclass-id.h>

/* Most devices one uclass can hold */
#define UCLASS_MAX_DEVICES 8


/**
 * Add a bound device to the uclass of its driver.
 *
 * Returns 0, or -EINVAL if the driver's uclass ID is not valid, -ENOMEM if
 * the uclass is full.
 */
int uclass_add_device(
    struct udevice *dev
);


/**
 * Get the device at position 'index' in a uclass, counting from 0 in the
 * order the devices were bound. The device is not probed.
 *
 * Returns 0 and stores the device in '*devp', or -EINVAL if 'id' is not
 * valid, -ENODEV if the uclass has no device at that index.
 */
int uclass_find_device(
    enum uclass_id id,
    int index,
    struct udevice **devp
);


/**
 * Print every uclass and the devices in it. For debugging.
 */
void uclass_print_devices(void);


/**
 * Like uclass_find_device(), and also probe the device, so what comes back
 * is ready for use. This is the call drivers' users make.
 *
 * Returns 0 and stores the device in '*devp', or the error from the lookup
 * or from the probe. '*devp' is left alone on failure.
 */
int uclass_get_device(
    enum uclass_id id,
    int index,
    struct udevice **devp
);

#endif /* MINIBOOT_DM_UCLASS_H */
