all:
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -Wall -g -O0 -c uart.c -o uart.o
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -nostdlib -T stm32f429.ld uart.o -o uart.elf -Wl,-Map=uart.map
	arm-none-eabi-objcopy -O binary uart.elf uart.bin

load:
	openocd -f board/stm32f429disc1.cfg -c "program uart.elf verify reset exit"

clean:
	rm -f uart.o uart.elf uart.bin

PHONY: all load
