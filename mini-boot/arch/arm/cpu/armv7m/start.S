#include <mach/irqs.h>

    .syntax unified
    .thumb

/*
 * Vector table. The linker script places .isr_vector at the start of
 * flash, where the core looks for it after reset:
 *   word 0 -> initial MSP, word 1 -> reset handler.
 */
    .section .isr_vector, "a", %progbits
    .global vector_table
vector_table:
    .word   _estack
    .word   reset_handler
    .word   nmi_handler
    .word   hardfault_handler
    .word   memmanage_handler
    .word   busfault_handler
    .word   usagefault_handler
    .word   0
    .word   0
    .word   0
    .word   0
    .word   svc_handler
    .word   debugmon_handler
    .word   0
    .word   pendsv_handler
    .word   systick_handler

    /* peripheral IRQs; the count comes from the SoC; none is used yet */
    .rept   NR_IRQS
    .word   default_handler
    .endr

/*
 * Reset handler: build the C environment, then call main().
 * The symbols come from link.ld.
 */
    .text
    .global reset_handler
    .type   reset_handler, %function
    .thumb_func
reset_handler:
    /* copy .data from flash to RAM */
    ldr     r0, =_sidata
    ldr     r1, =_sdata
    ldr     r2, =_edata
1:  cmp     r1, r2
    bhs     2f
    ldr     r3, [r0], #4
    str     r3, [r1], #4
    b       1b

    /* zero .bss */
2:  ldr     r1, =_sbss
    ldr     r2, =_ebss
    movs    r3, #0
3:  cmp     r1, r2
    bhs     4f
    str     r3, [r1], #4
    b       3b

4:  bl      main
5:  b       5b                  /* main() must not return */

/*
 * HardFault: hand the exception frame to C. Bit 2 of the EXC_RETURN value
 * in lr tells which stack the core pushed the frame on.
    .global hardfault_handler
    .type   hardfault_handler, %function
    .thumb_func
hardfault_handler:
    tst     lr, #4
    ite     eq
    mrseq   r0, msp
    mrsne   r0, psp
    b       hardfault_report
 */

/* Unexpected exception: park here so the debugger shows where we are */
    .type   default_handler, %function
    .thumb_func
default_handler:
    b       default_handler

/* Weak aliases: defining a handler with the same name in C overrides one */
    .macro  weak_handler name
    .weak   \name
    .thumb_set \name, default_handler
    .endm

    weak_handler nmi_handler
    weak_handler hardfault_handler
    weak_handler memmanage_handler
    weak_handler busfault_handler
    weak_handler usagefault_handler
    weak_handler svc_handler
    weak_handler debugmon_handler
    weak_handler pendsv_handler
    weak_handler systick_handler
