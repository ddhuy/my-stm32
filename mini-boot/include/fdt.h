#ifndef MINIBOOT_FDT_H
#define MINIBOOT_FDT_H

#include <types.h>


/*
 * A minimal, read-only reader of a flattened device tree (DT spec v17),
 * the blob dtc makes from a .dts. Nothing is allocated: every call walks
 * the blob in place.
 *
 * A node is named by its offset in the blob's structure block, an int; the
 * root is FDT_ROOT. A negative value is an error code (libfdt does the
 * same). Every function takes the blob, which fdt_check_header() must
 * have accepted.
 */
#define FDT_ROOT    0


/**
 * A big-endian 32-bit value at 'p' (a cell, as the blob stores it), in the
 * CPU's byte order. 'p' needs no alignment.
 */
u32 fdt32_ld(
    const void *p
);


/**
 * Check that 'fdt' starts with a device tree header this reader knows.
 *
 * Returns 0, -EINVAL if the magic number is wrong (no blob there), or
 * -ENOTSUP if the format is older than version 17.
 */
int fdt_check_header(
    const void *fdt
);


/**
 * The first child of 'node'.
 *
 * Returns its offset, or -ENOENT if 'node' has no children.
 */
int fdt_first_child(
    const void *fdt,
    int node
);


/**
 * The next node after 'node' with the same parent.
 *
 * Returns its offset, or -ENOENT if 'node' is its parent's last child (or
 * the root).
 */
int fdt_next_sibling(
    const void *fdt,
    int node
);


/**
 * The name of 'node', with its unit address ("gpio@40020000"); "" for the
 * root. Points into the blob.
 */
const char *fdt_get_name(
    const void *fdt,
    int node
);


/**
 * The value of the property 'name' of 'node', and its length in bytes in
 * '*lenp' (if 'lenp' is not NULL). Points into the blob; big-endian, as
 * the blob stores it.
 *
 * Returns NULL if 'node' has no such property. A property without a value
 * (spi-cpol;) returns a pointer and a length of 0.
 */
const void *fdt_getprop(
    const void *fdt,
    int node,
    const char *name,
    int *lenp
);


/**
 * Read the property 'name' of 'node' as one 32-bit cell, converted to the
 * CPU's byte order.
 *
 * Returns 0 and the value in '*val', -ENOENT if there is no such
 * property, or -EINVAL if it is not exactly 4 bytes long. '*val' is left
 * alone on failure.
 */
int fdt_read_u32(
    const void *fdt,
    int node,
    const char *name,
    u32 *val
);


/**
 * The first property of 'node', and the next one after the property at
 * 'prop'. A property is named by its offset, like a node.
 *
 * Return its offset, or -ENOENT if there is none (left).
 */
int fdt_first_prop(
    const void *fdt,
    int node
);

int fdt_next_prop(
    const void *fdt,
    int prop
);


/**
 * The value of the property at offset 'prop', its name in '*namep' and its
 * length in '*lenp' (either may be NULL). Points into the blob.
 */
const void *fdt_prop_value(
    const void *fdt,
    int prop,
    const char **namep,
    int *lenp
);


/**
 * The node whose "phandle" property is 'phandle' (what <&label> in a .dts
 * becomes). A linear search of the whole tree.
 *
 * Returns its offset, or -ENOENT if no node has it.
 */
int fdt_node_by_phandle(
    const void *fdt,
    u32 phandle
);


/**
 * The node at the absolute 'path' ("/spi@40015000/gyro@0"), each
 * component compared with the full node name, unit address included.
 *
 * Returns its offset, or -ENOENT if there is no such node.
 */
int fdt_path_offset(
    const void *fdt,
    const char *path
);


/**
 * The label of 'node' ("gpioa" for gpioa: gpio@40020000), from the
 * /__symbols__ node dtc writes when run with -@. The first one if the node
 * has several. Points into the blob.
 *
 * Returns NULL if the node has no label, or the blob no /__symbols__.
 */
const char *fdt_get_label(
    const void *fdt,
    int node
);


/**
 * Print the whole tree in dtc's text format (what dtc -I dtb -O dts
 * prints, without the /dts-v1/ line). For debugging.
 */
void fdt_print(
    const void *fdt
);


#endif /* MINIBOOT_FDT_H */
