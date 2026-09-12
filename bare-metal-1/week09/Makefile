TOOLCHAIN=/workspace/toolchain/arm-gnu-toolchain-13.3.rel1-x86_64-arm-none-linux-gnueabihf
SYSROOT=$(TOOLCHAIN)/arm-none-linux-gnueabihf/libc
CROSS_COMPILE=$(TOOLCHAIN)/bin/arm-none-linux-gnueabihf-
ARCH=arm

CC=$(CROSS_COMPILE)gcc
AS=$(CROSS_COMPILE)as
LD=$(CROSS_COMPILE)ld
OC=$(CROSS_COMPILE)objcopy
OD=$(CROSS_COMPILE)objdump
OS=$(CROSS_COMPILE)size

all:
	$(CC) -Wall -g -O0 -I./ hello.c -o hello

clean:
	rm -f hello

PHONY: all clean