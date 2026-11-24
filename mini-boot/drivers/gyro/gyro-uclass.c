#include <errno.h>
#include <gyro.h>
#include <systime.h>
#include <dm/device.h>
#include <dm/uclass.h>


/*
 * The gyroscope uclass: the generic half of every gyroscope driver. The
 * caller gets the rate in mdps and never learns how the sensor is
 * reached (on this board: SPI, as a child of its bus).
 */

/*
 * Calibration: samples thrown away while the sensor settles after power
 * on, samples averaged, and the time between two of them (about one
 * output period at the reset output rate, 95-100 Hz).
 */
#define GYRO_CAL_SKIP         5
#define GYRO_CAL_SAMPLES      32
#define GYRO_CAL_INTERVAL_MS  10


/**
 * The uclass's data about one gyroscope (dev->uclass_priv). Private to this
 * file: no driver reads or writes it, so every gyroscope driver gets the
 * calibration without a line of its own.
 *
 * offset: the rate the sensor reports at rest, in mdps; zero until the
 *         first calibration (calloc)
 */
struct gyro_uc_priv
{
    struct gyro_data offset;
};


/**
 * Before the driver's probe: a gyroscope that cannot be read is not one,
 * so the driver is refused before it touches the sensor.
 *
 * Returns 0, or -ENOSYS if the driver has no read operation.
 */
static int gyro_pre_probe(
    struct udevice *dev
)
{
    const struct gyro_ops *ops = dev->driver->ops;

    if (!ops || !ops->read)
        return -ENOSYS;

    return SUCCESS;
}


/**
 * After the driver's probe, with the sensor on: the board is at rest at
 * boot, so this is when to measure the offset.
 *
 * Returns 0 or the error from gyro_calibrate().
 */
static int gyro_post_probe(
    struct udevice *dev
)
{
    return gyro_calibrate(dev);
}


UCLASS_DRIVER(gyro) = {
    .name = "gyro",
    .id   = UCLASS_GYRO,

    .per_device_auto = sizeof(struct gyro_uc_priv),

    .pre_probe  = gyro_pre_probe,
    .post_probe = gyro_post_probe,
};


/**
 * Both the table and the entry are checked, as in the other uclasses. The
 * driver returns the raw rate; the uclass removes the offset.
 */
int gyro_read(
    struct udevice *dev,
    struct gyro_data *data
)
{
    const struct gyro_ops *ops = dev->driver->ops;
    const struct gyro_uc_priv *uc_priv = dev->uclass_priv;
    struct gyro_data raw;

    if (!ops || !ops->read)
        return -ENOSYS;

    /* Never probed: no offset yet */
    if (!uc_priv)
        return -ENODEV;

    int ret = ops->read(dev, &raw);

    if (ret)
        return ret;

    data->x = raw.x - uc_priv->offset.x;
    data->y = raw.y - uc_priv->offset.y;
    data->z = raw.z - uc_priv->offset.z;

    return SUCCESS;
}


/**
 * Each sample comes from the driver directly, not through gyro_read():
 * a second calibration would otherwise average what the old offset left
 * over (about 0) and replace the real offset with it. The sum is s32
 * (GYRO_CAL_SAMPLES times 250 dps is far below its range) and is divided
 * once, so no digit is lost per sample.
 */
int gyro_calibrate(
    struct udevice *dev
)
{
    const struct gyro_ops *ops = dev->driver->ops;
    struct gyro_uc_priv *uc_priv = dev->uclass_priv;
    struct gyro_data raw;
    s32 sum_x = 0;
    s32 sum_y = 0;
    s32 sum_z = 0;

    if (!ops || !ops->read)
        return -ENOSYS;

    if (!uc_priv)
        return -ENODEV;

    for (int i = 0; i < GYRO_CAL_SKIP + GYRO_CAL_SAMPLES; i++)
    {
        systime_mdelay(GYRO_CAL_INTERVAL_MS);

        /* Raw, not gyro_read(): the old offset must not be in the sum */
        int ret = ops->read(dev, &raw);

        if (ret)
            return ret;

        if (i < GYRO_CAL_SKIP)
            continue;

        sum_x += raw.x;
        sum_y += raw.y;
        sum_z += raw.z;
    }

    /* Only now, so a failed read leaves the old offset */
    uc_priv->offset.x = sum_x / GYRO_CAL_SAMPLES;
    uc_priv->offset.y = sum_y / GYRO_CAL_SAMPLES;
    uc_priv->offset.z = sum_z / GYRO_CAL_SAMPLES;

    return SUCCESS;
}
