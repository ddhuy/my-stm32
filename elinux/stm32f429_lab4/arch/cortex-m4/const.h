#ifndef ARCH_CONST_H
#define ARCH_CONST_H

/*
 * Constants usable from both C and linker scripts (as in Linux
 * include/uapi/linux/const.h). Linker scripts are run through the C
 * preprocessor with __ASSEMBLER__ defined, and the linker does not
 * accept C suffixes, so:
 *
 *     _AC(0x08020000, u)  ->  0x08020000u   in C
 *                         ->  0x08020000    in a linker script
 *
 * Headers included by a linker script must also wrap C-only code
 * (types, prototypes, #include <stdint.h>) in #ifndef __ASSEMBLER__.
 */
#ifdef __ASSEMBLER__
#define _AC(X, Y)   X
#else
#define __AC(X, Y)  (X##Y)
#define _AC(X, Y)   __AC(X, Y)
#endif

#endif
