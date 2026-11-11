ARCH       := arm
CPU        := armv7m
MACH       := stm32f4
CPUFLAGS   := -mcpu=cortex-m4 -mthumb
OPENOCDCFG := board/stm32f429disc1.cfg

SRCS += drivers/gpio/stm32_gpio.c
SRCS += board/$(BOARD)/board.c