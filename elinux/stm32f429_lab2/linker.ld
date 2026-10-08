ENTRY(Reset_Handler)

MEMORY
{
    FLASH (rx) : ORIGIN = 0x08000000, LENGTH = 2M
    RAM (rwx) : ORIGIN = 0x20000000, LENGTH = 192K
    SDRAM (rwx) : ORIGIN = 0xD0000000, LENGTH = 8M
}

_estack = ORIGIN(RAM) + LENGTH(RAM);

SECTIONS
{
    .isr_vector :
    {
        KEEP(*(.isr_vector))
    } > FLASH

    .text :
    {
        *(.text*) *(.rodata*) . = ALIGN(4);
    } > FLASH

    .data :
    {
        . = ALIGN(4); _sdata = .;
        *(.data*) . = ALIGN(4);
        _edata = .;
    } > RAM AT> FLASH

    _sidata = LOADADDR(.data);

    .bss (NOLOAD) :
    {
        . = ALIGN(4); _sbss = .;
        *(.bss*) *(COMMON) . = ALIGN(4);
        _ebss = .;
    } > RAM

    .sdram (NOLOAD) :
    {
        . = ALIGN(4);
        __sdram_start__ = .;

        *(.sdram)
        *(.sdram.*)

        . = ALIGN(4);
        __sdram_end__ = .;
    } > SDRAM
}
