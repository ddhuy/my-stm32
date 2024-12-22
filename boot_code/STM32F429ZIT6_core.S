/**
 * These instructions define attributes of our chip and
 * the assembly language we'll use:
 */
.syntax unified
.cpu cortex-m4
.fpu softvfp
.thumb


/* Global memory locations */
.global Reset_Handler


/*
 * The Reset handler. Called on reset.
 */
.type Reset_Handler, %function
Reset_Handler:
    // Set the stack pointer to the end of the stack.
    // The '_estack' value is defined in our linker script.
    ldr r0, =_estack
    mov sp, r0

    // Copy data from flash to RAM data init section.
    // R2 will store our progress along the sidata section.
    movs r0, #0
    // Load the start/end address of the data section,
    // and the start of the data init section
    ldr r1, =_sdata
    ldr r2, =_edata
    ldr r3, =_sidata
    b copy_sidata_loop

    copy_sidata:
        // Offset the data init section by our copy progress.
        ldr r4, [r3, r0]
        // Copy the current word into data, and increment
        str r4, [r1, r0]
        adds r0, r0, #4
    copy_sidata_loop:
        // Unless we've copied the whole data section, copy the
        // next word from sidata->data.
        adds r4, r0, r1
        cmp r4, r2
        bcc copy_sidata

    // Once we are done copying the data section into RAM,
    // move on to filling the BSS section with 0s
    movs r0, #0
    ldr r1, =_sbss
    ldr r2, =_ebss
    b reset_bss_loop

    // Fill the BSS segment with '0's.
    reset_bss:
        // Store a 0 and increment by a word
        str r0, [r1]
        adds r1, r1, #4

    reset_bss_loop:
        cmp r1, r2
        bcc reset_bss

    // Branch to the 'main' method.
    b main

.size Reset_Handler, .-Reset_Handler

