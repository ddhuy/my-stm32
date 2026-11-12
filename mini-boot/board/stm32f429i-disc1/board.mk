ARCH       := arm
CPU        := armv7m
MACH       := stm32f4
CPUFLAGS   := -mcpu=cortex-m4 -mthumb
OPENOCDCFG := board/stm32f429disc1.cfg

SRCS += board/$(BOARD)/board.c
SRCS += drivers/gpio/stm32_gpio.c
SRCS += drivers/serial/stm32_serial.c