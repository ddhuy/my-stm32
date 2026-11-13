INCLUDE "memory.ld"


ENTRY(reset_handler)


_estack = ORIGIN(RAM) + LENGTH(RAM);
_sidata = LOADADDR(.data);


SECTIONS
{
    .isr_vector :
    {
        KEEP(*(.isr_vector))
    } > FLASH

    .text : ALIGN(4)
    {
        *(.text*)
        *(.rodata*)
        . = ALIGN(4);
    } > FLASH

    .data : ALIGN(4)
    {
        _sdata = .;
        *(.data*)
        . = ALIGN(4);
        _edata = .;
    } > RAM AT> FLASH

    .bss : ALIGN(4)
    {
        _sbss = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;
    } > RAM

    /*
     * malloc() area: no contents in the image (NOLOAD). The size comes from
     * the board's memory.ld. The stack grows down from _estack towards it.
     */
    .heap (NOLOAD) : ALIGN(8)
    {
        _sheap = .;
        . += _heap_size;
        _eheap = .;
    } > RAM
}

/* Fail the link if the heap and the reserved stack do not both fit in RAM */
ASSERT(_eheap <= _estack - _stack_size, "RAM too small for heap & stack");
