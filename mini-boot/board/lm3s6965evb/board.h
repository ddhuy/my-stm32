#ifndef MINIBOOT_BOARD_LM3S6965EVB_H
#define MINIBOOT_BOARD_LM3S6965EVB_H

/*
 * Board facts: QEMU machine lm3s6965evb. Included by board.c only; drivers
 * never see this file.
 */
#include <mach/lm3s6965.h>
#include <platdata/lm3s_sysctl.h>


/* 8 MHz crystal on the main oscillator; run at the chip's maximum, 50 MHz */
#define BOARD_XTAL_HZ     8000000
#define BOARD_SYSCLK_HZ   50000000

/*
 * Console: UART0. On the real chip its pins (PA0 / PA1) would need GPIO
 * setup; QEMU models no pins.
 */
#define CONSOLE_UART_BASE (UART0_BASE)
#define CONSOLE_UART_CLK_DEV LM3S_SYSCTL_NAME
#define CONSOLE_UART_CLK  (LM3S_CLK_UART0)
#define CONSOLE_BAUDRATE  115200

#endif /* MINIBOOT_BOARD_LM3S6965EVB_H */
