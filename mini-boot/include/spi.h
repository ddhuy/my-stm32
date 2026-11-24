#ifndef MINIBOOT_SPI_H
#define MINIBOOT_SPI_H

#include <types.h>


struct udevice;


/*
 * Clock polarity and phase, named as in Linux. Mode 0: the clock idles
 * low, data is sampled on the rising edge. Mode 3: idles high, sampled on
 * the rising (second) edge.
 */
#define SPI_CPHA        BIT(0)  /* sample on the second clock edge */
#define SPI_CPOL        BIT(1)  /* clock idles high */
#define SPI_MODE_0      0
#define SPI_MODE_3      (SPI_CPOL | SPI_CPHA)
#define SPI_MODE_MASK   (SPI_CPOL | SPI_CPHA)   /* the only valid mode bits */

/* Transfer flags: where a transfer starts and ends (chip select) */
#define SPI_XFER_BEGIN  BIT(0)  /* set the bus up, CS low first */
#define SPI_XFER_END    BIT(1)  /* CS high after the last bit */


/**
 * How one slave wants to be talked to, as the board describes it. Defined
 * by the SPI uclass, not by a driver; only the SPI uclass reads it (in
 * child_pre_probe), and a slave's driver never does. It is the slave
 * device's parent_plat.
 *
 * cs:     chip select line on its bus
 * mode:   SPI_MODE_0 .. SPI_MODE_3
 * max_hz: fastest clock the slave accepts
 */
struct spi_slave_plat
{
    u32 cs;
    u32 mode;
    u32 max_hz;
};


/**
 * One slave at run time: its settings, copied from its spi_slave_plat by
 * the SPI uclass (child_pre_probe) once they are checked. It is the slave
 * device's parent_priv, and what a bus driver works with. A copy, not a
 * pointer: it is the live state, which may change at run time (a slave
 * that must start slow), while the board's plat stays as written.
 *
 * cs:     chip select line on its bus
 * mode:   SPI_MODE_0 .. SPI_MODE_3
 * max_hz: fastest clock the slave accepts, never 0
 */
struct spi_slave
{
    u32 cs;
    u32 mode;
    u32 max_hz;
};


/**
 * The operations every SPI bus driver implements.
 *
 * xfer: shift 'len' bytes out of 'dout' and in to 'din' (either may be
 *       NULL) for 'slave' on 'bus'; 'flags' are SPI_XFER_*. Returns 0 or
 *       -Exxx.
 */
struct spi_ops
{
    int (*xfer) (struct udevice *bus, const struct spi_slave *slave,
                 const u8 *dout, u8 *din, size_t len, u32 flags);
};


/**
 * Shift 'len' bytes out of 'dout' and in to 'din' (either may be NULL)
 * for the SPI slave device 'slave'. Its bus is its parent, its settings
 * its parent_priv (struct spi_slave). 'flags' are SPI_XFER_*: a register
 * read is two calls, the address with SPI_XFER_BEGIN and the data with
 * SPI_XFER_END.
 *
 * Returns 0, or -ENODEV if 'slave' has no parent, -EINVAL if the parent
 * is not an SPI bus or 'slave' has no settings (not probed as an SPI
 * slave), -ENOSYS if the bus driver has no xfer operation, or the bus
 * driver's error.
 */
int spi_xfer(
    struct udevice *slave,
    const u8 *dout,
    u8 *din,
    size_t len,
    u32 flags
);


#endif /* MINIBOOT_SPI_H */
