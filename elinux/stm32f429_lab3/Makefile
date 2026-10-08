PREFIX ?= arm-none-eabi-
CC = $(PREFIX)gcc
OBJDUMP = $(PREFIX)objdump
OBJCOPY = $(PREFIX)objcopy
SIZE = $(PREFIX)size
CFLAGS = -mcpu=cortex-m4 -mthumb -mfloat-abi=soft -std=gnu11 -O1 -g3 -ffreestanding -fno-builtin -fno-lto -fno-unwind-tables -fno-asynchronous-unwind-tables -ffunction-sections -fdata-sections -Wall -Wextra
LDFLAGS = -mcpu=cortex-m4 -mthumb -mfloat-abi=soft -nostdlib -Wl,--gc-sections -Wl,-Map=firmware.map -T linker.ld
all: firmware.elf firmware.bin firmware.lst
firmware.elf: startup.o main.o linker.ld
	$(CC) $(LDFLAGS) startup.o main.o -o $@
	$(SIZE) $@
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
firmware.bin: firmware.elf
	$(OBJCOPY) -O binary $< $@
firmware.lst: firmware.elf
	$(OBJDUMP) -d -S $< > $@
flash: firmware.elf
	openocd -f interface/stlink.cfg -f target/stm32f4x.cfg -c 'program firmware.elf verify reset exit'
clean:
	rm -f *.o *.elf *.bin *.lst *.map
.PHONY: all flash clean
