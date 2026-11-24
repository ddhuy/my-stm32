/*
 * The board's device tree blob, built by dtc from board/<board>/<board>.dts
 * and put into flash as it is. DTB_FILE is set by the Makefile. The spec
 * asks for 8-byte alignment.
 */
	.section .rodata.dtb, "a"
	.balign 8
	.global _dtb_start
_dtb_start:
	.incbin DTB_FILE
	.global _dtb_end
_dtb_end:
