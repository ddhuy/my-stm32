#ifndef MINIBOOT_BOARD_LM3S6965EVB_H
#define MINIBOOT_BOARD_LM3S6965EVB_H

/*
 * Board facts: QEMU machine lm3s6965evb. Included by board.c only; drivers
 * never see this file.
 */
#include <platdata/lm3s_sysctl.h>

/*
 * The devices themselves (addresses, crystal, speeds) are in
 * lm3s6965evb.dts. This file keeps what board.c needs.
 */

/*
 * Console: UART0, for the banner's clock line. On the real chip its pins
 * (PA0 / PA1) would need GPIO setup; QEMU models no pins.
 */
#define CONSOLE_UART_CLK_DEV LM3S_SYSCTL_NAME
#define CONSOLE_UART_CLK  (LM3S_CLK_UART0)

#endif /* MINIBOOT_BOARD_LM3S6965EVB_H */
