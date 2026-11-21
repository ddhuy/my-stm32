#ifndef MINIBOOT_BOARD_LM3S6965EVB_H
#define MINIBOOT_BOARD_LM3S6965EVB_H

/*
 * Board facts: QEMU machine lm3s6965evb. Included by board.c only; drivers
 * never see this file.
 */

#include <mach/clock_ids.h>
#include <mach/lm3s6965.h>


/*
 * Console: UART0. On the real chip its pins (PA0 / PA1) would need GPIO
 * setup; QEMU models no pins.
 */
#define CONSOLE_UART_BASE (UART0_BASE)
#define CONSOLE_UART_CLK  (CLK_UART0)
#define CONSOLE_BAUDRATE  115200

#endif /* MINIBOOT_BOARD_LM3S6965EVB_H */
