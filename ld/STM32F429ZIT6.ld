/* Label for the program's entry point */
ENTRY(Reset_Handler)


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
	FLASH (rx) : ORIGIN = 0x08000000, LENGTH = 2M
	SRAM (xrw) : ORIGIN = 0x20000000, LENGTH = 192K
}


SECTIONS
{
	/* The vector table goes at the start of flash. */
	.isr_vector :
	{
		. = ALIGN(4);
		KEEP (*(.isr_vector))
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
	.data : AT(_sidata)
	{
		. = ALIGN(4);
		/* Mark start/end locations for the 'data' section. */
		_sdata = .;
		*(.data)
		*(.data*)
		_edata = .;
		. = ALIGN(4);
	} >SRAM


	/* The 'bss' section is similar to the 'data' section,
	 * but its space is initialied to all 0s at the start
	 * of the program.
	 */
	.bss :
	{
		. = ALIGN(4);
		/* Also mark the start/end of the BSS section. */
		_sbss = .;
		*(.bss)
		*(.bss*)
		*(COMMON)
		. = ALIGN(4);
		_ebss = .;
	} >SRAM

	
	/* Space set aside for application's heap/stack
	 */
	.dynamic_allocations :
	{
		. = ALIGN(4);
		_ssystem_ram = .;
		. = . + _Min_Heap_Size;
		. = . + _Min_Stack_Size;
		. = ALIGN(4);
		_esystem_ram = .;
	} >SRAM
}
