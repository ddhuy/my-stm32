TOOLCHAIN = /workspace/toolchain/arm-gnu-toolchain-13.3.rel1-x86_64-arm-none-eabi
CC = $(TOOLCHAIN)/bin/arm-none-eabi-gcc
AS = $(TOOLCHAIN)/bin/arm-none-eabi-as
LD = $(TOOLCHAIN)/bin/arm-none-eabi-ld
OC = $(TOOLCHAIN)/bin/arm-none-eabi-objcopy
OD = $(TOOLCHAIN)/bin/arm-none-eabi-objdump
OS = $(TOOLCHAIN)/bin/arm-none-eabi-size


all:
	$(CC) -mcpu=cortex-m4 -mthumb -Wall -g -O0 -I./ -c gyro.c -o gyro.o
	$(CC) -mcpu=cortex-m4 -mthumb -nostdlib -T stm32f429.ld gyro.o -o gyro.elf -Wl,-Map=gyro.map
	$(OC) -O binary gyro.elf gyro.bin

load:
	openocd -f board/stm32f429disc1.cfg -c "program gyro.elf verify reset exit"

clean:
	rm -f gyro.o gyro.elf gyro.bin gyro.map

PHONY: all load
