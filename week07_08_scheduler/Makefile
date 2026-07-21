TOOLCHAIN = /workspace/toolchain/arm-gnu-toolchain-13.3.rel1-x86_64-arm-none-eabi
CC = $(TOOLCHAIN)/bin/arm-none-eabi-gcc
AS = $(TOOLCHAIN)/bin/arm-none-eabi-as
LD = $(TOOLCHAIN)/bin/arm-none-eabi-ld
OC = $(TOOLCHAIN)/bin/arm-none-eabi-objcopy
OD = $(TOOLCHAIN)/bin/arm-none-eabi-objdump
OS = $(TOOLCHAIN)/bin/arm-none-eabi-size


all:
	$(CC) -mcpu=cortex-m4 -mthumb -Wall -g -O0 -I./ -c pendsv.S -o pendsv.o
	$(CC) -mcpu=cortex-m4 -mthumb -Wall -g -O0 -I./ -c scheduler.c -o scheduler.o
	$(CC) -mcpu=cortex-m4 -mthumb -nostdlib -T stm32f429.ld scheduler.o pendsv.o -o scheduler.elf -Wl,-Map=scheduler.map
	$(OC) -O binary scheduler.elf scheduler.bin

load:
	openocd -f board/stm32f429disc1.cfg -c "program scheduler.elf verify reset exit"

clean:
	rm -f scheduler.o pendsv.o scheduler.elf scheduler.bin scheduler.map

PHONY: all load
