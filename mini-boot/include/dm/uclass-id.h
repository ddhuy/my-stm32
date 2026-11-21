#ifndef MINIBOOT_DM_UCLASS_ID_H
#define MINIBOOT_DM_UCLASS_ID_H


/**
 * The kinds of device the driver model knows. Every driver names the one it
 * belongs to. The value is also the index into the core's table of uclasses,
 * so UCLASS_COUNT must stay last.
 */
enum uclass_id
{
    UCLASS_INVALID = 0,
    UCLASS_GPIO,
    UCLASS_SERIAL,
    UCLASS_TIMER,
    UCLASS_CLK,

    UCLASS_COUNT
};


#endif /* MINIBOOT_DM_UCLASS_ID_H */
