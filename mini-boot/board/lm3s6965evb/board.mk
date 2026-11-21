ARCH         := arm
CPU          := armv7m
MACH         := lm3s
CPUFLAGS     := -mcpu=cortex-m3 -mthumb
QEMU_MACHINE := lm3s6965evb

SRCS += board/$(BOARD)/board.c
SRCS += drivers/serial/pl011_serial.c
