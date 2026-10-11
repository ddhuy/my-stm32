ENTRY(reset_handler)
MEMORY
{
    SDRAM (rwx) : ORIGIN = 0xD0000000, LENGTH = (8 * 1024 * 1024)
    RAM (rwx) : ORIGIN = 0x20000000, LENGTH = (128 * 1024)
}
__stack_size = 4K;
__estack = ORIGIN(RAM) + LENGTH(RAM);
__stack_limit = __estack - __stack_size;
SECTIONS
{
    .isr_vector : ALIGN(512)
    {
        __vector_table_start__ = .;
        KEEP(*(.isr_vector))
        . = ALIGN(4);
        __vector_table_end__ = .;
    } > SDRAM
    .text :
    {
        . = ALIGN(4);
        *(.text*)
        *(.rodata*)
        . = ALIGN(4);
    } > SDRAM
    .data :
    {
        . = ALIGN(4);
        __sdata = .;
        *(.data*)
        . = ALIGN(4);
        __edata = .;
    } > RAM AT> SDRAM
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
ASSERT(ADDR(.isr_vector) == ORIGIN(SDRAM),
       "ERROR: Application vector table must start at SDRAM base")
ASSERT(__ebss <= __stack_limit,
       "ERROR: Application SRAM data overlaps reserved stack")
