#ifndef MINIBOOT_GYRO_H
#define MINIBOOT_GYRO_H

#include <types.h>


struct udevice;


/**
 * Angular rate about each axis, in millidegrees per second (mdps). The
 * unit is part of the uclass contract: every gyroscope driver converts
 * its raw counts, so a caller never knows the sensor's scale. The axes
 * are the sensor's, which need not be the board's.
 */
struct gyro_data
{
    s32 x;
    s32 y;
    s32 z;
};


/**
 * The operations every gyroscope driver implements.
 *
 * read: the latest rate on all three axes; returns 0 or -Exxx
 */
struct gyro_ops
{
    int (*read) (struct udevice *dev, struct gyro_data *data);
};


/**
 * Read the latest angular rate from the gyroscope 'dev' (a probed device
 * of UCLASS_GYRO), less the offset gyro_calibrate() measured, so a
 * gyroscope at rest reads about 0. 'data' is filled in only on success.
 *
 * Returns 0, -ENOSYS if the driver has no read operation, -ENODEV if the
 * device was never probed (no uclass data, no offset), or the driver's
 * error.
 */
int gyro_read(
    struct udevice *dev,
    struct gyro_data *data
);


/**
 * Measure the zero-rate offset of 'dev' now, from the average of raw
 * samples, and keep it for gyro_read(). The device must be still while
 * this runs (about 0.4 s): a turn is taken as part of the offset. Done
 * once at probe by the uclass (post_probe); call it again to recalibrate.
 * Needs systime (systime_mdelay() never returns before systime_init()).
 *
 * Returns 0, -ENOSYS if the driver has no read operation, -ENODEV if the
 * device was never probed, or the driver's error; on failure the old
 * offset is kept.
 */
int gyro_calibrate(
    struct udevice *dev
);

#endif /* MINIBOOT_GYRO_H */
