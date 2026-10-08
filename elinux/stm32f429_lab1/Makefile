TOOLCHAIN = /workspace/toolchain/arm-gnu-toolchain-13.3.rel1-x86_64-arm-none-eabi
CC = $(TOOLCHAIN)/bin/arm-none-eabi-gcc
AS = $(TOOLCHAIN)/bin/arm-none-eabi-as
LD = $(TOOLCHAIN)/bin/arm-none-eabi-ld
OC = $(TOOLCHAIN)/bin/arm-none-eabi-objcopy
OD = $(TOOLCHAIN)/bin/arm-none-eabi-objdump
OS = $(TOOLCHAIN)/bin/arm-none-eabi-size


CFLAGS=-mcpu=cortex-m4 -mthumb -mfloat-abi=soft -std=gnu11 -O0 -g3 -ffreestanding -fno-builtin -Wall -Wextra -ffunction-sections -fdata-sections
LDFLAGS=-T linker.ld -nostdlib -Wl,--gc-sections -Wl,-Map=firmware.map


all: firmware.bin firmware.lst

firmware.elf: main.c linker.ld
	$(CC) $(CFLAGS) main.c $(LDFLAGS) -o $@

firmware.bin: firmware.elf
	$(OC) -O binary $< $@

firmware.lst: firmware.elf
	$(OD) -d -S $< > $@

clean:
	rm -f firmware.elf firmware.bin firmware.lst firmware.map

.PHONY: all clean
