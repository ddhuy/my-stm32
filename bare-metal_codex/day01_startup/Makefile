all:
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -Wall -g -O0 -c main.c -o main.o
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -nostdlib -T stm32f429.ld main.o -o day01_startup.elf -Wl,-Map=day01_startup.map
	arm-none-eabi-objcopy -O binary day01_startup.elf day01_startup.bin

load:
	openocd -f board/stm32f429disc1.cfg -c "program day01_startup.elf verify reset exit"

clean:
	rm -f main.o day01_startup.elf day01_startup.bin day01_startup.map

.PHONY: all load clean
