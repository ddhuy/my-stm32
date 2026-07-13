all:
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -Wall -g -O0 -c blink.c -o blink.o
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -nostdlib -T stm32f429.ld blink.o -o blink.elf -Wl,-Map=blink.map
	arm-none-eabi-objcopy -O binary blink.elf blink.bin

load:
	openocd -f board/stm32f429disc1.cfg -c "program blink.elf verify reset exit"

clean:
	rm -f blink.o blink.elf blink.bin

PHONY: all load
