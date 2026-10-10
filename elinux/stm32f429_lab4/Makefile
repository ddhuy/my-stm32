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

BUILD_DIR := build/application

APP_SOURCE_DIR := application/src
APP_INCLUDE_DIR:= application/include
APP_LD_SCRIPT  := application/linker.ld

APP_SOURCES := $(wildcard $(APP_SOURCE_DIR)/*.c)
APP_OBJECTS := $(patsubst $(APP_SOURCE_DIR)/%.c,$(BUILD_DIR)/%.o,$(APP_SOURCES))
APP_ELF     := $(BUILD_DIR)/application.elf
APP_BIN     := $(BUILD_DIR)/application.bin

CFLAGS := -mcpu=cortex-m4 -mthumb -O0 -g3 -Wall -Wextra \
          -ffreestanding -fno-builtin -ffunction-sections -fdata-sections \
          -I$(APP_INCLUDE_DIR)

LDFLAGS := -T $(APP_LD_SCRIPT) -nostdlib \
           -Wl,--gc-sections \
           -Wl,-Map=$(BUILD_DIR)/application.map

.PHONY: all clean inspect

$(APP_ELF): $(APP_OBJECTS) $(APP_LD_SCRIPT)
	$(CC) $(CFLAGS) $(APP_OBJECTS) $(LDFLAGS) -o $@
	$(OBJCOPY) -O binary $@ $(APP_BIN)

$(BUILD_DIR)/%.o: $(APP_SOURCE_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $@

all: $(APP_ELF)

clean:
	rm -rf $(BUILD_DIR)

inspect: all
	$(SIZE) $(APP_ELF)
	$(OBJDUMP) -h $(APP_ELF)
	$(READELF) -l $(APP_ELF)
