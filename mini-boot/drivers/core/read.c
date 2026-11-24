#include <errno.h>
#include <fdt.h>
#include <dm/device.h>
#include <dm/read.h>


/*
 * The blob, linked by common/dtb.S. An array: the symbol is an address.
 */
extern const char _dtb_start[];

/* Default cell counts of a node without #address-cells / #size-cells */
#define FDT_DEFAULT_ADDR_CELLS  2
#define FDT_DEFAULT_SIZE_CELLS  1

#define FDT_CELL_SIZE           4


/**
 * The header is checked on every call: it is two loads, and nothing has
 * to remember the answer.
 */
const void *dm_fdt(void)
{
    if (fdt_check_header(_dtb_start))
        return NULL;

    return _dtb_start;
}


/**
 * See dm/read.h.
 */
const void *dev_read_prop(
    const struct udevice *dev,
    const char *name,
    int *lenp
)
{
    const void *fdt = dm_fdt();

    if (!fdt)
        return NULL;

    return fdt_getprop(fdt, dev->node, name, lenp);
}


/**
 * See dm/read.h.
 */
int dev_read_u32(
    const struct udevice *dev,
    const char *name,
    u32 *val
)
{
    const void *fdt = dm_fdt();

    if (!fdt)
        return -ENODEV;

    return fdt_read_u32(fdt, dev->node, name, val);
}


/**
 * Present with any length is true: a flag has none.
 */
bool dev_read_bool(
    const struct udevice *dev,
    const char *name
)
{
    return dev_read_prop(dev, name, NULL) != NULL;
}


/**
 * A cell count of the parent node, or the spec's default without one.
 */
static u32 parent_cells(
    const void *fdt,
    int parent,
    const char *name,
    u32 dflt
)
{
    u32 val;

    if (fdt_read_u32(fdt, parent, name, &val))
        return dflt;

    return val;
}


/**
 * The cell counts are the parent node's: a reg says nothing about its own
 * layout.
 */
int dev_read_addr_index(
    const struct udevice *dev,
    int index,
    uintptr_t *addr
)
{
    const void *fdt = dm_fdt();

    if (!fdt)
        return -ENODEV;

    int parent = dev->parent ? dev->parent->node : FDT_ROOT;
    u32 ac = parent_cells(fdt, parent, "#address-cells",
                          FDT_DEFAULT_ADDR_CELLS);
    u32 sc = parent_cells(fdt, parent, "#size-cells",
                          FDT_DEFAULT_SIZE_CELLS);

    if (ac != 1)
        return -EINVAL;

    int len;
    const u8 *reg = fdt_getprop(fdt, dev->node, "reg", &len);
    int entry = (ac + sc) * FDT_CELL_SIZE;

    if (!reg || index < 0 || (index + 1) * entry > len)
        return -ENOENT;

    *addr = fdt32_ld(reg + index * entry);

    return SUCCESS;
}


/**
 * Each entry is a phandle and then as many cells as the node it points to
 * declares, so the list is walked from its start up to 'index'.
 */
int dev_read_phandle_with_args(
    const struct udevice *dev,
    const char *list_name,
    const char *cells_name,
    int index,
    struct fdt_phandle_args *out
)
{
    const void *fdt = dm_fdt();

    if (!fdt)
        return -ENODEV;

    int len;
    const u8 *list = fdt_getprop(fdt, dev->node, list_name, &len);

    if (!list)
        return -ENOENT;

    int cells = len / FDT_CELL_SIZE;
    int i = 0;

    for (int entry = 0; i < cells; entry++)
    {
        int node = fdt_node_by_phandle(fdt, fdt32_ld(list +
                                                    i * FDT_CELL_SIZE));
        u32 count;

        if (node < 0)
            return -ENOENT;

        if (fdt_read_u32(fdt, node, cells_name, &count) ||
            count > FDT_MAX_PHANDLE_ARGS)
            return -EINVAL;

        if (i + 1 + (int)count > cells)
            return -ENOENT;

        if (entry == index)
        {
            out->node = node;
            out->count = count;
            for (u32 c = 0; c < count; c++)
                out->args[c] = fdt32_ld(list + (i + 1 + c) *
                                        FDT_CELL_SIZE);
            return SUCCESS;
        }

        i += 1 + count;
    }

    return -ENOENT;
}
