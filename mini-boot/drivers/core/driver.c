#include <printf.h>
#include <string.h>
#include <dm/driver.h>


/**
 * Start and end of the driver array, set by the linker script around the
 * .dm_drivers section. Declared as arrays because the symbols are addresses:
 * a pointer declaration would make the compiler load a value stored there.
 */
extern const struct driver _sdm_drivers[];
extern const struct driver _edm_drivers[];


/**
 * Linear search; there are only a handful of drivers.
 */
const struct driver *driver_find(
    const char *name
)
{
    for (const struct driver *drv = _sdm_drivers;
         drv < _edm_drivers;
         ++drv)
    {
        if ( !strcmp(drv->name, name) )
            return drv;
    }

    return NULL;
}


/**
 * Every driver's table, entry by entry, until the NULL compatible.
 */
const struct driver *driver_find_by_compatible(
    const char *compatible,
    uintptr_t *datap
)
{
    for (const struct driver *drv = _sdm_drivers;
         drv < _edm_drivers;
         ++drv)
    {
        if (!drv->of_match)
            continue;

        for (const struct udevice_id *id = drv->of_match;
             id->compatible;
             ++id)
        {
            if (!strcmp(id->compatible, compatible))
            {
                *datap = id->data;
                return drv;
            }
        }
    }

    return NULL;
}


/**
 * Print all available drivers.
 */
void driver_print_list(void)
{
    int index = 0;

    /* The count is a pointer subtraction, not bytes. */
    printf("Total drivers: %d\n", _edm_drivers - _sdm_drivers);

    for (const struct driver *drv = _sdm_drivers;
         drv < _edm_drivers;
         ++drv)
    {
        printf("[%d] Driver: %s.\n"
               "    UClass: %u.\n"
               "    Private Data Size: %u.\n",
            index++, drv->name, drv->id, drv->priv_auto);
    }
}
