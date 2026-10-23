ENTRY(reset_handler)
MEMORY
{
    FLASH (rx) : ORIGIN = 0x08000000, LENGTH = 128K
    RAM (rwx) : ORIGIN = 0x20000000, LENGTH = 128K
}
__stack_size = 4K;
__estack = ORIGIN(RAM) + LENGTH(RAM);
__stack_limit = __estack - __stack_size;
SECTIONS
{
    .isr_vector :
    {
        . = ALIGN(512);
        KEEP(*(.isr_vector))
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
    ASSERT(ADDR(.isr_vector) == ORIGIN(FLASH),
        "ERROR: Bootloader vector table must start at the flash base")
    ASSERT(__ebss <= __stack_limit,
        "ERROR: Bootloader SRAM data overlaps reserved stack")
}
