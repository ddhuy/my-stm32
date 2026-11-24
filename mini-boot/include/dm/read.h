#ifndef MINIBOOT_DM_READ_H
#define MINIBOOT_DM_READ_H

#include <types.h>


struct udevice;


/*
 * Reading a device's device tree node, for a driver's of_to_plat (U-Boot's
 * dev_read_*). Every function reads 'dev->node' in the board's blob; if
 * the blob is not valid they fail with -ENODEV, or return NULL / false.
 */

/* Most argument cells a phandle reference may have (<&gpioc 1 0>: 2) */
#define FDT_MAX_PHANDLE_ARGS  4


/**
 * One entry of a phandle list (clocks, cs-gpios): the node it points to,
 * and the cells that follow the phandle, as many as that node's
 * #clock-cells / #gpio-cells says.
 */
struct fdt_phandle_args
{
    int node;
    int count;
    u32 args[FDT_MAX_PHANDLE_ARGS];
};


/**
 * The board's device tree blob, or NULL if the image's blob has no valid
 * header.
 */
const void *dm_fdt(void);


/**
 * The value of the property 'name' of the device's node, its length in
 * '*lenp' (if not NULL). NULL if absent (or no valid blob).
 */
const void *dev_read_prop(
    const struct udevice *dev,
    const char *name,
    int *lenp
);


/**
 * The property 'name' as one cell.
 *
 * Returns 0 and the value in '*val'; -ENODEV without a blob, -ENOENT if
 * absent, -EINVAL if not exactly one cell.
 */
int dev_read_u32(
    const struct udevice *dev,
    const char *name,
    u32 *val
);


/**
 * Whether the device's node has the property 'name' (spi-cpol;).
 */
bool dev_read_bool(
    const struct udevice *dev,
    const char *name
);


/**
 * The address of entry 'index' of the node's reg. The cell counts come
 * from the parent node (#address-cells, #size-cells; 2 and 1 if absent,
 * as the spec says): the root for a top-level device. Only one-cell
 * (32-bit) addresses are accepted.
 *
 * Returns 0 and the address in '*addr'; -ENODEV without a blob; -ENOENT
 * without reg or past its end; -EINVAL if an address is not one cell.
 */
int dev_read_addr_index(
    const struct udevice *dev,
    int index,
    uintptr_t *addr
);


/**
 * Entry 'index' of the phandle list 'list_name' (clocks = <&rcc 164>,
 * <&clk_hse>): the node it points to, and its arguments, whose number is
 * the 'cells_name' property (#clock-cells) of that node. The entries may
 * have different lengths, so the list is walked from the start.
 *
 * Returns 0 and the entry in '*out'; -ENODEV without a blob; -ENOENT if
 * the list is absent, too short, or a phandle points nowhere; -EINVAL if
 * that node has no 'cells_name' or more than FDT_MAX_PHANDLE_ARGS.
 */
int dev_read_phandle_with_args(
    const struct udevice *dev,
    const char *list_name,
    const char *cells_name,
    int index,
    struct fdt_phandle_args *out
);


#endif /* MINIBOOT_DM_READ_H */
