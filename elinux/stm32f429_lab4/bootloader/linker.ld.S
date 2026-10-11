/*
 * Bootloader linker script. Run through the C preprocessor by the Makefile
 * (build/bootloader/linker.ld), so addresses come from memory_map.h.
 * The FLASH region ends where the application image starts, so a bootloader
 * that grows too big fails to link instead of overwriting the image.
 */
#include "memory_map.h"

ENTRY(reset_handler)


MEMORY
{
    FLASH (rx)  : ORIGIN = BOOT_FLASH_BASE, LENGTH = BOOT_FLASH_SIZE
    RAM   (rwx) : ORIGIN = BOOT_SRAM_BASE,  LENGTH = BOOT_SRAM_SIZE
}


__stack_size = 4K;

__estack = ORIGIN(RAM) + LENGTH(RAM);
__stack_limit = __estack - __stack_size;


SECTIONS
{
    .isr_vector :
    {
        KEEP(*(.isr_vector))
        . = ALIGN(4);
    } > FLASH

    .text :
    {
        . = ALIGN(4);
        *(.text*)
        *(.rodata*)
        . = ALIGN(4);
    } > FLASH

    .data :
    {
        . = ALIGN(4);
        __sdata = .;
        *(.data*)
        . = ALIGN(4);
        __edata = .;
    } > RAM AT> FLASH

    __sidata = LOADADDR(.data);

    .bss (NOLOAD) :
    {
        . = ALIGN(4);
        __sbss = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        __ebss = .;
    } > RAM
}


ASSERT(ADDR(.isr_vector) == ORIGIN(FLASH),
       "ERROR: Bootloader vector table must start at the flash base")

ASSERT(__ebss <= __stack_limit,
       "ERROR: Bootloader SRAM data overlaps reserved stack")
