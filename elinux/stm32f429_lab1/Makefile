CC=arm-none-eabi-gcc
OBJCOPY=arm-none-eabi-objcopy
OBJDUMP=arm-none-eabi-objdump
CFLAGS=-mcpu=cortex-m4 -mthumb -mfloat-abi=soft -std=gnu11 -O0 -g3 -ffreestanding -fno-builtin -Wall -Wextra -ffunction-sections -fdata-sections
LDFLAGS=-T linker.ld -nostdlib -Wl,--gc-sections -Wl,-Map=firmware.map
all: firmware.bin firmware.lst
firmware.elf: startup.c main.c linker.ld
	$(CC) $(CFLAGS) startup.c main.c $(LDFLAGS) -o $@
firmware.bin: firmware.elf
	$(OBJCOPY) -O binary $< $@
firmware.lst: firmware.elf
	$(OBJDUMP) -d -S $< > $@

load:
	openocd -f board/stm32f429disc1.cfg -c "program firmware.elf verify reset exit"

clean:
	rm -f firmware.elf firmware.bin firmware.lst firmware.map

.PHONY: all clean load
