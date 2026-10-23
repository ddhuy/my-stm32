ENTRY(reset_handler)
MEMORY {
  FLASH (rx) : ORIGIN = 0x08000000, LENGTH = 2048K
  RAM (rwx) : ORIGIN = 0x20000000, LENGTH = 192K
  SDRAM (rwx) : ORIGIN = 0xD0000000, LENGTH = 8M
}
_estack = ORIGIN(RAM) + LENGTH(RAM);
SECTIONS {
  .isr_vector : { . = ALIGN(4); KEEP(*(.isr_vector)) . = ALIGN(4); } > FLASH
  .text : { . = ALIGN(4); *(.text .text.*) *(.rodata .rodata.*) . = ALIGN(4); } > FLASH
  /DISCARD/ : { *(.ARM.exidx* .gnu.linkonce.armexidx.*) }
  .sdram_text : ALIGN(4) {
    __sdram_text_start__ = .;
    KEEP(*(.sdram_text .sdram_text.*))
    . = ALIGN(4);
    __sdram_text_end__ = .;
  } > SDRAM AT> FLASH
  __sdram_text_load__ = LOADADDR(.sdram_text);
  .data : ALIGN(4) {
    _sdata = .; *(.data .data.*) . = ALIGN(4); _edata = .;
  } > RAM AT> FLASH
  _sidata = LOADADDR(.data);
  .bss (NOLOAD) : ALIGN(4) {
    _sbss = .; *(.bss .bss.*) *(COMMON) . = ALIGN(4); _ebss = .;
  } > RAM
  . = ALIGN(8);
  ASSERT(SIZEOF(.sdram_text) > 0, "Missing SDRAM code")
  ASSERT(__sdram_text_end__ <= ORIGIN(SDRAM) + LENGTH(SDRAM), "SDRAM overflow")
}
