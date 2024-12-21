/* Label for the program's entry point */
ENTRY(reset_handler)


/* End of RAM -> Start of Stack */
/* Define the end of RAM and limit of stack memory
 * (192KB SRAM + 64KB CCM) == 256KB SRAM on the STM32F429 line
 * 192KB = 0x30000
 * RAM starts at address 0x20000000
 */
_estack = 0x20030000;    /* end of 192K RAM */


/* Set minimum size for stack and dynamic memory */
/* (The linker will generate an error if there is 
 * less than this much RAM leftover.)
 * (1KB)
 * Generate a link error if heap and stack don't fit into RAM
 */
_Min_Heap_Size = 0;      /* required amount of heap  */
_Min_Stack_Size = 0x400; /* required amount of stack */


MEMORY
{
	FLASH (rx)     : ORIGIN = 0x08000000, LENGTH = 2M
	RAM (xrw)      : ORIGIN = 0x20000000, LENGTH = 192K
	MEMORY_B1 (rx) : ORIGIN = 0x60000000, LENGTH = 0K
	CCMRAM (rw)    : ORIGIN = 0x10000000, LENGTH = 64K
}


SECTIONS
{
	/* The vector table goes at the start of flash. */
	.vector_table :
	{
		. = ALIGN(4);
		KEEP (*(.vector_table))
		. = ALIGN(4);
	} >FLASH

	/* The 'text' section contains the main program code */
	.text :
	{
		. = ALIGN(4);
		*(.text)
		*(.text*)
		. = ALIGN(4);
		_etext = .;
	} >FLASH

	/* The 'rodata' section contains read-only data,
	 * constants, strings, information that won't change.
	 */
	.rodata :
	{
		. = ALIGN(4);
		*(.rodata)
		*(.rodata*)
		. = ALIGN(4);
	} >FLASH


	/* The 'data' section is space set aside in RAM for
	 * things like variables, which can change.
	 */
	_sidata = .;
	.data :
	{
		. = ALIGN(4);
		/* Mark start/end locations for the 'data' section. */
		_sdata = .;
		*(.data)
		*(.data*)
		. = ALIGN(4);
		_edata = .;
	} >RAM AT> FLASH


	/* The 'bss' section is similar to the 'data' section,
	 * but its space is initialied to all 0s at the start
	 * of the program.
	 */
	. = ALIGN(4);
	.bss :
	{
		/* Also mark the start/end of the BSS section. */
		_sbss = .;
		__bss_start__ = _sbss;
		*(.bss)
		*(.bss*)
		*(COMMON)
		. = ALIGN(4);
		_ebss = .;
		__bss_end__ = _ebss;
	} >RAM

	
	/* Space set aside for application's heap/stack
	 */
	.dynamic_allocations :
	{
		. = ALIGN(4);
		PROVIDE ( end = . );
		PROVIDE ( _end = . );
		. = . + _Min_Heap_Size;
		. = . + _Min_Stack_Size;
		. = ALIGN(4);
	} >RAM
}
