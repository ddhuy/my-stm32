#include <errno.h>
#include <fdt.h>
#include <printf.h>
#include <string.h>
#include <types.h>


/*
 * The blob's layout (DT spec, "Flattened devicetree format"). Every
 * number in it is big-endian, so each is read byte by byte with
 * fdt32_ld(): that works on any CPU and at any alignment.
 *
 * Header: ten 32-bit fields, at these byte offsets.
 */
#define FDT_MAGIC               0xd00dfeed
#define FDT_VERSION_MIN         17        /* has size_dt_struct */

#define FDT_OFF_MAGIC           0
#define FDT_OFF_TOTALSIZE       4
#define FDT_OFF_DT_STRUCT       8
#define FDT_OFF_DT_STRINGS      12
#define FDT_OFF_VERSION         20
#define FDT_OFF_SIZE_DT_STRUCT  36

/*
 * The structure block is a stream of 32-bit tokens:
 *   BEGIN_NODE, the name (NUL-terminated), padding to 4
 *   PROP, len, nameoff (into the strings block), len bytes, padding to 4
 *   END_NODE, NOP, END: the token alone
 */
#define FDT_BEGIN_NODE          0x1
#define FDT_END_NODE            0x2
#define FDT_PROP                0x3
#define FDT_NOP                 0x4
#define FDT_END                 0x9

#define FDT_TAG_SIZE            4
#define FDT_PROP_HDR_SIZE       8         /* len, nameoff */
#define FDT_ALIGN(x)            (((x) + 3) & ~3)

/* Bytes per cell when printing a value as <...> */
#define FDT_CELL_SIZE           4


/**
 * Byte by byte, so 'p' needs no alignment and the CPU's byte order does
 * not matter.
 */
u32 fdt32_ld(
    const void *p
)
{
    const u8 *b = p;

    return ((u32)b[0] << 24) | ((u32)b[1] << 16) | ((u32)b[2] << 8) | b[3];
}


/**
 * A header field.
 */
static u32 fdt_header(
    const void *fdt,
    int field
)
{
    return fdt32_ld((const u8 *)fdt + field);
}


/**
 * The address of 'offset' in the structure block.
 */
static const u8 *fdt_struct(
    const void *fdt,
    int offset
)
{
    return (const u8 *)fdt + fdt_header(fdt, FDT_OFF_DT_STRUCT) + offset;
}


/**
 * The tag at 'offset', and in '*next' the offset just past it: past the
 * name of a node, past the value of a property, both padded to 4. Never
 * reads beyond the structure block: an offset outside it, or a token
 * that does not fit in it, reads as FDT_END, so a bad blob stops a walk
 * instead of running through flash.
 */
static u32 fdt_next_tag(
    const void *fdt,
    int offset,
    int *next
)
{
    u32 size = fdt_header(fdt, FDT_OFF_SIZE_DT_STRUCT);

    if (offset < 0 || (u32)offset + FDT_TAG_SIZE > size)
        return FDT_END;

    u32 tag = fdt32_ld(fdt_struct(fdt, offset));
    u32 end = offset + FDT_TAG_SIZE;

    switch (tag)
    {
    case FDT_BEGIN_NODE:
    {
        /* The name, NUL included, inside the block */
        const u8 *name = fdt_struct(fdt, end);
        u32 len = 0;

        while (end + len < size && name[len])
            len++;
        if (end + len >= size)
            return FDT_END;

        end += len + 1;
        break;
    }

    case FDT_PROP:
        if (end + FDT_PROP_HDR_SIZE > size)
            return FDT_END;

        end += FDT_PROP_HDR_SIZE + fdt32_ld(fdt_struct(fdt, end));
        if (end > size)
            return FDT_END;
        break;

    case FDT_END_NODE:
    case FDT_NOP:
        break;

    default:
        return FDT_END;
    }

    *next = FDT_ALIGN(end);

    return tag;
}


/**
 * The first tag after 'offset' that is not a NOP, and its offset in
 * '*at'.
 */
static u32 fdt_skip_nops(
    const void *fdt,
    int offset,
    int *at
)
{
    u32 tag;
    int next;

    for (;;)
    {
        tag = fdt_next_tag(fdt, offset, &next);
        if (tag != FDT_NOP)
            break;
        offset = next;
    }

    *at = offset;

    return tag;
}


/**
 * The offset of the first tag after the BEGIN_NODE of 'node' and all its
 * properties: its first child, or its END_NODE. FDT_END's offset on a bad
 * blob.
 */
static int fdt_after_props(
    const void *fdt,
    int node
)
{
    int offset;

    /* Past the BEGIN_NODE and the name */
    if (fdt_next_tag(fdt, node, &offset) != FDT_BEGIN_NODE)
        return -EINVAL;

    for (;;)
    {
        int at;
        u32 tag = fdt_skip_nops(fdt, offset, &at);

        if (tag != FDT_PROP)
            return at;

        fdt_next_tag(fdt, at, &offset);
    }
}


/**
 * The name of a property from its PROP tag at 'prop': nameoff counts from
 * the start of the strings block, not the structure block.
 */
static const char *fdt_prop_name(
    const void *fdt,
    int prop
)
{
    u32 nameoff = fdt32_ld(fdt_struct(fdt, prop + FDT_TAG_SIZE +
                                      FDT_TAG_SIZE));

    return (const char *)fdt + fdt_header(fdt, FDT_OFF_DT_STRINGS) +
           nameoff;
}


/**
 * Only the magic number and the version are checked: the blob comes from
 * our own build, and every walk is bounded by size_dt_struct anyway.
 */
int fdt_check_header(
    const void *fdt
)
{
    if (fdt_header(fdt, FDT_OFF_MAGIC) != FDT_MAGIC)
        return -EINVAL;

    if (fdt_header(fdt, FDT_OFF_VERSION) < FDT_VERSION_MIN)
        return -ENOTSUP;

    return SUCCESS;
}


/**
 * After the properties comes either a child's BEGIN_NODE or the node's
 * own END_NODE.
 */
int fdt_first_child(
    const void *fdt,
    int node
)
{
    int at;

    if (fdt_skip_nops(fdt, fdt_after_props(fdt, node), &at) !=
        FDT_BEGIN_NODE)
        return -ENOENT;

    return at;
}


/**
 * Walk to the END_NODE that closes 'node', counting the depth of its
 * subtree on the way. What follows is a sibling (BEGIN_NODE) or the end
 * of the parent (END_NODE).
 */
int fdt_next_sibling(
    const void *fdt,
    int node
)
{
    int depth = 0;
    int offset = node;

    do
    {
        int next;

        switch (fdt_next_tag(fdt, offset, &next))
        {
        case FDT_BEGIN_NODE:
            depth++;
            break;
        case FDT_END_NODE:
            depth--;
            break;
        case FDT_END:
            return -ENOENT;
        default:
            break;
        }

        offset = next;
    } while (depth > 0);

    int at;

    if (fdt_skip_nops(fdt, offset, &at) != FDT_BEGIN_NODE)
        return -ENOENT;

    return at;
}


/**
 * The name follows the BEGIN_NODE tag.
 */
const char *fdt_get_name(
    const void *fdt,
    int node
)
{
    return (const char *)fdt_struct(fdt, node + FDT_TAG_SIZE);
}


/**
 * A linear search of the node's own properties; a child's are never
 * reached, because the walk stops at the first tag that is not a PROP.
 */
const void *fdt_getprop(
    const void *fdt,
    int node,
    const char *name,
    int *lenp
)
{
    int offset;

    if (fdt_next_tag(fdt, node, &offset) != FDT_BEGIN_NODE)
        return NULL;

    for (;;)
    {
        int at;

        if (fdt_skip_nops(fdt, offset, &at) != FDT_PROP)
            return NULL;

        if (!strcmp(fdt_prop_name(fdt, at), name))
        {
            const u8 *prop = fdt_struct(fdt, at + FDT_TAG_SIZE);

            if (lenp)
                *lenp = fdt32_ld(prop);

            return prop + FDT_PROP_HDR_SIZE;
        }

        fdt_next_tag(fdt, at, &offset);
    }
}


/**
 * The length is checked, so a string or a list of cells is not read as
 * one number.
 */
int fdt_read_u32(
    const void *fdt,
    int node,
    const char *name,
    u32 *val
)
{
    int len;
    const void *prop = fdt_getprop(fdt, node, name, &len);

    if (!prop)
        return -ENOENT;

    if (len != FDT_CELL_SIZE)
        return -EINVAL;

    *val = fdt32_ld(prop);

    return SUCCESS;
}


/**
 * The first PROP after the BEGIN_NODE of 'node' (NOPs skipped), or the
 * first tag that ends the properties.
 */
int fdt_first_prop(
    const void *fdt,
    int node
)
{
    int offset;
    int at;

    if (fdt_next_tag(fdt, node, &offset) != FDT_BEGIN_NODE)
        return -ENOENT;

    if (fdt_skip_nops(fdt, offset, &at) != FDT_PROP)
        return -ENOENT;

    return at;
}


/**
 * Properties come before the children, so the first tag that is not a
 * PROP ends them.
 */
int fdt_next_prop(
    const void *fdt,
    int prop
)
{
    int offset;
    int at;

    if (fdt_next_tag(fdt, prop, &offset) != FDT_PROP)
        return -ENOENT;

    if (fdt_skip_nops(fdt, offset, &at) != FDT_PROP)
        return -ENOENT;

    return at;
}


/**
 * The PROP tag is followed by len and nameoff, then the value.
 */
const void *fdt_prop_value(
    const void *fdt,
    int prop,
    const char **namep,
    int *lenp
)
{
    const u8 *hdr = fdt_struct(fdt, prop + FDT_TAG_SIZE);

    if (namep)
        *namep = fdt_prop_name(fdt, prop);
    if (lenp)
        *lenp = fdt32_ld(hdr);

    return hdr + FDT_PROP_HDR_SIZE;
}


/**
 * Every BEGIN_NODE in the block, in order; a node without a "phandle"
 * property is simply not a match.
 */
int fdt_node_by_phandle(
    const void *fdt,
    u32 phandle
)
{
    int offset = FDT_ROOT;

    for (;;)
    {
        int next;
        u32 tag = fdt_next_tag(fdt, offset, &next);
        u32 val;

        if (tag == FDT_END)
            return -ENOENT;

        if (tag == FDT_BEGIN_NODE &&
            !fdt_read_u32(fdt, offset, "phandle", &val) && val == phandle)
            return offset;

        offset = next;
    }
}


/**
 * One component at a time, from the root down: each is looked for among
 * the children of the node found so far.
 */
int fdt_path_offset(
    const void *fdt,
    const char *path
)
{
    int node = FDT_ROOT;

    if (*path != '/')
        return -ENOENT;

    while (*path)
    {
        while (*path == '/')
            path++;
        if (!*path)
            break;

        size_t len = 0;

        while (path[len] && path[len] != '/')
            len++;

        int child;

        for (child = fdt_first_child(fdt, node);
             child >= 0;
             child = fdt_next_sibling(fdt, child))
        {
            const char *name = fdt_get_name(fdt, child);

            if (!strncmp(name, path, len) && name[len] == '\0')
                break;
        }

        if (child < 0)
            return -ENOENT;

        node = child;
        path += len;
    }

    return node;
}


/**
 * Each property of /__symbols__ is a label, its value the path of the
 * labelled node; the path is resolved and compared by offset.
 */
const char *fdt_get_label(
    const void *fdt,
    int node
)
{
    int symbols = fdt_path_offset(fdt, "/__symbols__");

    if (symbols < 0)
        return NULL;

    for (int prop = fdt_first_prop(fdt, symbols);
         prop >= 0;
         prop = fdt_next_prop(fdt, prop))
    {
        const char *label;
        const char *path = fdt_prop_value(fdt, prop, &label, NULL);

        if (fdt_path_offset(fdt, path) == node)
            return label;
    }

    return NULL;
}


/**
 * dtc's rule (util_is_printable_string): ends with a NUL, every other
 * byte printable or a NUL between two non-empty strings.
 */
static bool fdt_is_string(
    const u8 *val,
    int len
)
{
    if (len == 0 || val[len - 1] != '\0' || val[0] == '\0')
        return false;

    for (int i = 0; i < len - 1; i++)
    {
        if (val[i] == '\0')
        {
            if (val[i + 1] == '\0')
                return false;
        }
        else if (val[i] < ' ' || val[i] > '~')
            return false;
    }

    return true;
}


/**
 * One property, as dtc prints it: no value, strings, cells, or bytes.
 */
static void fdt_print_prop(
    const char *name,
    const u8 *val,
    int len
)
{
    if (len == 0)
    {
        printf("%s;\n", name);
        return;
    }

    printf("%s = ", name);

    if (fdt_is_string(val, len))
    {
        printf("\"%s\"", (const char *)val);

        for (int i = 0; i < len - 1; i++)
            if (val[i] == '\0')
                printf(", \"%s\"", (const char *)&val[i + 1]);
    }
    else if (len % FDT_CELL_SIZE == 0)
    {
        printf("<");
        for (int i = 0; i < len; i += FDT_CELL_SIZE)
            printf("%s0x%02x", i ? " " : "", fdt32_ld(&val[i]));
        printf(">");
    }
    else
    {
        printf("[");
        for (int i = 0; i < len; i++)
            printf("%s%02x", i ? " " : "", val[i]);
        printf("]");
    }

    printf(";\n");
}


/**
 * One linear walk over the tokens, the depth giving the indentation (a tab
 * per level, as dtc). A blank line before each node but the first one of
 * its parent's that has no property before it, as dtc does.
 */
void fdt_print(
    const void *fdt
)
{
    int offset = FDT_ROOT;
    int depth = 0;
    bool blank = false;

    for (;;)
    {
        int next;
        u32 tag = fdt_next_tag(fdt, offset, &next);

        if (tag == FDT_END)
            break;

        switch (tag)
        {
        case FDT_BEGIN_NODE:
        {
            const char *name = fdt_get_name(fdt, offset);

            if (blank)
                printf("\n");
            for (int i = 0; i < depth; i++)
                printf("\t");
            printf("%s {\n", depth ? name : "/");
            depth++;
            blank = false;
            break;
        }

        case FDT_PROP:
        {
            const u8 *prop = fdt_struct(fdt, offset + FDT_TAG_SIZE);

            for (int i = 0; i < depth; i++)
                printf("\t");
            fdt_print_prop(fdt_prop_name(fdt, offset), prop +
                           FDT_PROP_HDR_SIZE, fdt32_ld(prop));
            blank = true;
            break;
        }

        case FDT_END_NODE:
            depth--;
            for (int i = 0; i < depth; i++)
                printf("\t");
            printf("};\n");
            blank = true;
            break;

        default:
            break;
        }

        offset = next;
    }
}
