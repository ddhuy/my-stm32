#include <asm/armv7m.h>
#include <asm/io.h>
#include <panic.h>
#include <printf.h>
#include <types.h>


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
 * Called from hardfault_handler in start.S with the exception frame
 */
void hardfault_report(const u32 *frame)
{
    printf("\nHardFault\n");
    printf("  pc   = %08x\n", frame[FRAME_PC]);
    printf("  lr   = %08x\n", frame[FRAME_LR]);
    printf("  cfsr = %08x\n", readl(SCB_BASE + SCB_CFSR));
    printf("  hfsr = %08x\n", readl(SCB_BASE + SCB_HFSR));

    panic("unrecoverable fault");
}
