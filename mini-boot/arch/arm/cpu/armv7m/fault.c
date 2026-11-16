#include <asm/armv7m.h>
#include <asm/io.h>
#include <panic.h>
#include <printf.h>
#include <types.h>


/*
 * Called only from start.S, so no header declares it. The prototype is here
 * for -Wmissing-prototypes; the compiler cannot check it against the
 * assembly caller.
 */
void hardfault_report(
    const u32 *frame
);


/**
 * Registers the core pushes on the interrupted stack at exception entry,
 * lowest address first.
 */
enum {
    FRAME_R0,
    FRAME_R1,
    FRAME_R2,
    FRAME_R3,
    FRAME_R12,
    FRAME_LR,
    FRAME_PC,
    FRAME_XPSR,
};


/**
 * Called from hardfault_handler in start.S, which passes the address of the
 * exception frame in r0 (the main or the process stack, whichever was in
 * use when the fault happened).
 */
void hardfault_report(
    const u32 *frame
)
{
    printf("\nHardFault\n");
    printf("  pc   = %08x\n", frame[FRAME_PC]);
    printf("  lr   = %08x\n", frame[FRAME_LR]);
    printf("  cfsr = %08x\n", readl(SCB_BASE + SCB_CFSR));
    printf("  hfsr = %08x\n", readl(SCB_BASE + SCB_HFSR));

    panic("unrecoverable fault");
}
