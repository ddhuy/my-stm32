#include <errno.h>
#include <gyro.h>
#include <spi.h>
#include <types.h>
#include <dm/device.h>
#include <dm/driver.h>


/*
 * ST L3GD20 three-axis gyroscope on SPI, and the I3G4250D that replaces it
 * on newer boards (same registers, different WHO_AM_I). The device is a
 * child of its SPI bus: every transfer goes through spi_xfer(), which
 * finds the bus and the chip select, so this driver never names either.
 *
 * Register access (datasheet, SPI): the first byte is the register
 * address, with bit 7 set for a read and bit 6 to auto-increment it.
 */
#define L3GD20_WHO_AM_I      0x0F
#define L3GD20_CTRL_REG1     0x20
#define L3GD20_CTRL_REG4     0x23
#define L3GD20_OUT_X_L       0x28        /* X, Y, Z: low byte first */
#define L3GD20_ADDR_READ     BIT(7)
#define L3GD20_ADDR_MS       BIT(6)      /* auto-increment the address */
#define CTRL_REG1_XEN        BIT(0)
#define CTRL_REG1_YEN        BIT(1)
#define CTRL_REG1_ZEN        BIT(2)
#define CTRL_REG1_PD         BIT(3)      /* power on (normal mode) */
/*
 * CTRL_REG4, every bit chosen here, so it is written whole:
 * - BDU: block data update; the output registers keep a sample until both
 *   of its bytes are read, so a read never mixes two samples.
 * - BLE 0: low byte at the lower address (what the read assembles).
 * - FS 00: +-250 dps, the range L3GD20_MDPS_PER_DIGIT_X100 is for.
 * - self-test off.
 * - SIM 0: 4-wire SPI. With SIM 1 (3-wire) the sensor answers on SDI and
 *   leaves SDO, our MISO, floating: every read gives 0xFF. A sensor left
 *   in that state by an earlier boot survives an MCU reset, but still
 *   receives writes, so probe writes CTRL_REG4 before its first read.
 */
#define CTRL_REG4_BDU        BIT(7)
#define CTRL_REG4_BLE        BIT(6)
#define CTRL_REG4_FS_SHIFT   4
#define CTRL_REG4_FS_MASK    (0x3 << CTRL_REG4_FS_SHIFT)
#define CTRL_REG4_FS_250DPS  (0x0 << CTRL_REG4_FS_SHIFT)
#define CTRL_REG4_SIM        BIT(0)
#define CTRL_REG4_VAL        (CTRL_REG4_BDU | CTRL_REG4_FS_250DPS)

/*
 * The bits the read-back compares. SIM is not among them: a sensor in
 * 3-wire mode could not answer the read at all (0xFF fails the FS check).
 */
#define CTRL_REG4_CHECK      (CTRL_REG4_BDU | CTRL_REG4_BLE | \
                              CTRL_REG4_FS_MASK)
#define ID_L3GD20            0xD4
#define ID_I3G4250D          0xD3        /* on this board */


/**
 * One chip of the family this driver handles: what its match-table entry
 * hands the device as driver_data.
 *
 * name:     for the debugger
 * who_am_i: what its WHO_AM_I register reads
 */
struct l3gd20_chip
{
    const char *name;
    u8 who_am_i;
};

static const struct l3gd20_chip l3gd20_chip = {
    .name     = "L3GD20",
    .who_am_i = ID_L3GD20,
};

static const struct l3gd20_chip i3g4250d_chip = {
    .name     = "I3G4250D",
    .who_am_i = ID_I3G4250D,
};

/* Sensitivity at +-250 dps: 8.75 mdps per digit, as 875 / 100 */
#define L3GD20_MDPS_PER_DIGIT_X100  875
#define L3GD20_SCALE_DIV            100

/* Three axes of two bytes each */
#define L3GD20_AXES          3
#define L3GD20_OUT_LEN       (L3GD20_AXES * 2)

/* Normal mode, all three axes on; output rate and bandwidth at reset */
#define CTRL_REG1_ON  (CTRL_REG1_PD | CTRL_REG1_XEN | CTRL_REG1_YEN | \
                       CTRL_REG1_ZEN)


/**
 * Read 'len' registers from 'reg' on: the address with SPI_XFER_BEGIN,
 * then the data with SPI_XFER_END, so CS stays low between the two.
 * (More than one register also needs L3GD20_ADDR_MS in 'reg'.)
 *
 * Returns 0 or the error from spi_xfer().
 */
static int l3gd20_read(
    struct udevice *dev,
    u8 reg,
    u8 *buf,
    size_t len
)
{
    u8 addr = reg | L3GD20_ADDR_READ;
    int ret = spi_xfer(dev, &addr, NULL, 1, SPI_XFER_BEGIN);

    if (ret)
        return ret;

    return spi_xfer(dev, NULL, buf, len, SPI_XFER_END);
}


/**
 * Write one register: address (read bit clear) and value in one
 * transfer.
 *
 * Returns 0 or the error from spi_xfer().
 */
static int l3gd20_write(
    struct udevice *dev,
    u8 reg,
    u8 val
)
{
    const u8 out[] = { reg, val };

    return spi_xfer(dev, out, NULL, sizeof(out),
                    SPI_XFER_BEGIN | SPI_XFER_END);
}


/**
 * Set the sensor up and check that it is one this driver knows: CTRL_REG4
 * first (4-wire SPI, BDU, +-250 dps), because a sensor left in 3-wire mode
 * cannot answer a read until it is written; then WHO_AM_I; then power on
 * with all three axes. BDU before CTRL_REG1: once on, the sensor samples,
 * and the first read must not mix two samples. The core has already
 * probed the bus (this device's parent), so the first transfer can go
 * straight out.
 *
 * The device tree says which chip it is (driver_data, from the matched
 * compatible string), so WHO_AM_I must be that chip's.
 *
 * Returns 0; the error from the bus; -ENODEV if WHO_AM_I is not the
 * expected chip's (another chip, or none: an absent chip reads as 0x00 or
 * 0xFF); -EIO if CTRL_REG4 (the CTRL_REG4_CHECK bits) or CTRL_REG1 does
 * not read back what was written.
 */
static int l3gd20_probe(
    struct udevice *dev
)
{
    const struct l3gd20_chip *chip = (const void *)dev->driver_data;
    u8 val;
    int ret;

    /* Before any read: brings a sensor in 3-wire mode back to 4-wire */
    ret = l3gd20_write(dev, L3GD20_CTRL_REG4, CTRL_REG4_VAL);
    if (ret)
        return ret;

    ret = l3gd20_read(dev, L3GD20_WHO_AM_I, &val, 1);
    if (ret)
        return ret;
    if (val != chip->who_am_i)
        return -ENODEV;

    ret = l3gd20_read(dev, L3GD20_CTRL_REG4, &val, 1);
    if (ret)
        return ret;
    if ((val & CTRL_REG4_CHECK) != CTRL_REG4_VAL)
        return -EIO;

    ret = l3gd20_write(dev, L3GD20_CTRL_REG1, CTRL_REG1_ON);
    if (ret)
        return ret;

    ret = l3gd20_read(dev, L3GD20_CTRL_REG1, &val, 1);
    if (ret)
        return ret;
    if (val != CTRL_REG1_ON)
        return -EIO;

    return SUCCESS;
}


/**
 * All six output bytes in one transaction (MS: the address increments).
 * Each axis is two's complement, low byte first: the s16 cast makes a
 * negative rate negative. Multiply before dividing, so no digit is lost;
 * an s16 times 875 fits an s32.
 *
 * Returns 0 or the error from the bus; 'data' is filled in only on
 * success.
 */
static int l3gd20_get_rate(
    struct udevice *dev,
    struct gyro_data *data
)
{
    u8 buf[L3GD20_OUT_LEN];
    s32 rate[L3GD20_AXES];

    int ret = l3gd20_read(dev, L3GD20_OUT_X_L | L3GD20_ADDR_MS, buf,
                          sizeof(buf));
    if (ret)
        return ret;

    for (int i = 0; i < L3GD20_AXES; i++) {
        s16 raw = (s16)(buf[2 * i] | (buf[2 * i + 1] << 8));

        rate[i] = (s32)raw * L3GD20_MDPS_PER_DIGIT_X100 / L3GD20_SCALE_DIV;
    }

    data->x = rate[0];
    data->y = rate[1];
    data->z = rate[2];

    return SUCCESS;
}


/**
 * This driver's implementation of the gyroscope uclass operations.
 */
static const struct gyro_ops l3gd20_ops = {
    .read = l3gd20_get_rate,
};


/**
 * The compatible strings this driver handles, one per chip (Linux's), each
 * with its chip as driver_data.
 */
static const struct udevice_id l3gd20_ids[] = {
    { .compatible = "st,l3gd20-gyro",   .data = (uintptr_t)&l3gd20_chip },
    { .compatible = "st,i3g4250d-gyro", .data = (uintptr_t)&i3g4250d_chip },
    { }
};

/**
 * Driver model entry. No platform data and no private data. The device's
 * parent_plat (struct spi_slave_plat) is read by the SPI uclass, never
 * here.
 */
DM_DRIVER(l3gd20) = {
    .name  = "l3gd20",
    .id    = UCLASS_GYRO,
    .of_match = l3gd20_ids,

    .probe = l3gd20_probe,

    .ops = &l3gd20_ops,
};
