TOOLCHAIN = /workspace/toolchain/arm-gnu-toolchain-13.3.rel1-x86_64-arm-none-eabi
CC = $(TOOLCHAIN)/bin/arm-none-eabi-gcc
AS = $(TOOLCHAIN)/bin/arm-none-eabi-as
LD = $(TOOLCHAIN)/bin/arm-none-eabi-ld
OC = $(TOOLCHAIN)/bin/arm-none-eabi-objcopy
OD = $(TOOLCHAIN)/bin/arm-none-eabi-objdump
OS = $(TOOLCHAIN)/bin/arm-none-eabi-size


all:
	$(CC) -mcpu=cortex-m4 -mthumb -Wall -g -O0 -I./ -c interrupts.c -o interrupts.o
	$(CC) -mcpu=cortex-m4 -mthumb -nostdlib -T stm32f429.ld interrupts.o -o interrupts.elf -Wl,-Map=interrupts.map
	$(OC) -O binary interrupts.elf interrupts.bin

load:
	openocd -f board/stm32f429disc1.cfg -c "program interrupts.elf verify reset exit"

clean:
	rm -f interrupts.o interrupts.elf interrupts.bin interrupts.map

PHONY: all load
