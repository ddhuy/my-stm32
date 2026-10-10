TOOLCHAIN_PATH = /workspace/toolchain/arm-gnu-toolchain-13.3.rel1-x86_64-arm-none-eabi
CROSS_COMPILER = $(TOOLCHAIN_PATH)/bin/arm-none-eabi-

CC      := $(CROSS_COMPILER)gcc
CXX     := $(CROSS_COMPILER)g++
AS      := $(CROSS_COMPILER)as
LD      := $(CROSS_COMPILER)ld
SIZE    := $(CROSS_COMPILER)size
OBJCOPY := $(CROSS_COMPILER)objcopy
OBJDUMP := $(CROSS_COMPILER)objdump
READELF := $(CROSS_COMPILER)readelf

APP_BUILD_DIR  := build/application
BOOT_BUILD_DIR := build/bootloader

BOOTLOADER_SOURCE_DIR := bootloader/src
BOOTLOADER_INCLUDE_DIR := bootloader/include

COMMON_SOURCE_DIR  := common/src
COMMON_INCLUDE_DIR := common/include

APP_SOURCE_DIR := application/src
APP_INCLUDE_DIR:= application/include
APP_LD_SCRIPT  := application/linker.ld

APP_SOURCES := $(wildcard $(APP_SOURCE_DIR)/*.c)
APP_OBJECTS := $(patsubst $(APP_SOURCE_DIR)/%.c,$(APP_BUILD_DIR)/%.o,$(APP_SOURCES))
APP_ELF     := $(APP_BUILD_DIR)/application.elf
APP_BIN     := $(APP_BUILD_DIR)/application.bin

BOOT_SOURCES := $(wildcard $(BOOTLOADER_SOURCE_DIR)/*.c)
BOOT_OBJECTS := $(patsubst $(BOOTLOADER_SOURCE_DIR)/%.c,$(BOOT_BUILD_DIR)/%.o,$(BOOT_SOURCES))
BOOT_ELF     := $(BOOT_BUILD_DIR)/bootloader.elf
BOOT_BIN     := $(BOOT_BUILD_DIR)/bootloader.bin

CFLAGS := -mcpu=cortex-m4 -mthumb -O0 -g3 -Wall -Wextra \
          -ffreestanding -fno-builtin -ffunction-sections -fdata-sections \
          -I$(APP_INCLUDE_DIR) -I$(COMMON_INCLUDE_DIR) -I$(BOOTLOADER_INCLUDE_DIR)

LDFLAGS := -T $(APP_LD_SCRIPT) -nostdlib \
           -Wl,--gc-sections \
           -Wl,-Map=$(APP_BUILD_DIR)/application.map

.PHONY: all clean inspect

$(APP_ELF): $(APP_OBJECTS)
	$(CC) $(CFLAGS) $(APP_OBJECTS) $(LDFLAGS) -o $@
	$(OBJCOPY) -O binary $@ $(APP_BIN)

$(APP_BUILD_DIR)/%.o: $(APP_SOURCE_DIR)/%.c | $(APP_BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(APP_BUILD_DIR):
	mkdir -p $@

all: $(APP_ELF)

clean:
	rm -rf $(APP_BUILD_DIR)

inspect: all
	$(SIZE) $(APP_ELF)
	$(OBJDUMP) -h $(APP_ELF)
	$(READELF) -l $(APP_ELF)
