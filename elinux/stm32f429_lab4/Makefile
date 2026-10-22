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
PYTHON  := python3

# Layers, like U-Boot: arch (CPU core) <- soc (chip) <- board (PCB).
# Code includes "arch/...", "soc/..." and "board.h"; these variables pick which.
ARCH  ?= cortex-m4
SOC   ?= stm32f429
BOARD ?= stm32f429i-disc1

BUILD_DIR      := build
APP_BUILD_DIR  := $(BUILD_DIR)/application
BOOT_BUILD_DIR := $(BUILD_DIR)/bootloader
TEST_BUILD_DIR := $(BUILD_DIR)/tests

# Shared by both programs: the layers, the drivers and the system contract
LAYER_INCLUDES := -I.

ARCH_FLAGS := -mcpu=cortex-m4 -mthumb

BASE_CFLAGS := $(ARCH_FLAGS) -O0 -g3 -Wall -Wextra \
               -ffreestanding -fno-builtin -ffunction-sections -fdata-sections \
               -MMD -MP

# Each program sees only its own private headers, never the other's
APP_INCLUDES  := -Iapplication/include $(LAYER_INCLUDES)
BOOT_INCLUDES := -Ibootloader/include $(LAYER_INCLUDES)
TEST_INCLUDES := -Ibootloader/include -Iapplication/include $(LAYER_INCLUDES)

APP_CFLAGS  := $(BASE_CFLAGS) $(APP_INCLUDES)
BOOT_CFLAGS := $(BASE_CFLAGS) $(BOOT_INCLUDES)
TEST_CFLAGS := -std=c11 -O2 -Wall -Wextra -Werror

BASE_LDFLAGS := $(ARCH_FLAGS) -nostdlib -Wl,--gc-sections

# Linker scripts are written as linker.ld.S and run through the C preprocessor,
# so they take their addresses from memory_map.h (see arch/const.h for _AC()).
LDS_CPP := $(CC) -E -P -x assembler-with-cpp -MMD -MP

# Value of a C macro from the shared headers, as a decimal number.
cpp_value = $(shell printf '%d' $$(( $$(echo '$(1)' | \
    $(CC) -E -P -x assembler-with-cpp $(LAYER_INCLUDES) \
    -include common/include/memory_map.h \
	-include common/include/image_format.h -) )))


# ---- Application: runs from SDRAM, packaged with a header for the bootloader
APP_SOURCES := $(wildcard application/src/*.c) \
			   $(wildcard board/$(BOARD)/*.c) \
			   $(wildcard drivers/*/*.c) \
			   $(shell find lib/ -name "*.c")
APP_OBJECTS := $(patsubst %.c,$(APP_BUILD_DIR)/obj/%.o,$(APP_SOURCES))
APP_LDS     := $(APP_BUILD_DIR)/linker.ld
APP_ELF     := $(APP_BUILD_DIR)/application.elf
APP_LST     := $(APP_BUILD_DIR)/application.lst
APP_BIN     := $(APP_BUILD_DIR)/application.bin
APP_IMG     := $(APP_BUILD_DIR)/application.img

# ---- Bootloader: runs from flash at 0x08000000
BOOT_SOURCES := $(wildcard bootloader/src/*.c) \
                $(wildcard board/$(BOARD)/*.c) \
                $(wildcard drivers/*/*.c) \
				$(shell find lib/ -name "*.c")
BOOT_OBJECTS := $(patsubst %.c,$(BOOT_BUILD_DIR)/obj/%.o,$(BOOT_SOURCES))
BOOT_LDS     := $(BOOT_BUILD_DIR)/linker.ld
BOOT_ELF     := $(BOOT_BUILD_DIR)/bootloader.elf
BOOT_LST     := $(BOOT_BUILD_DIR)/bootloader.lst
BOOT_BIN     := $(BOOT_BUILD_DIR)/bootloader.bin

# ---- Tests
TEST_SOURCES  := $(wildcard tests/test_*.c)
TEST_BINARIES := $(patsubst tests/%.c,build/tests/%,$(TEST_SOURCES))
TEST_RUNNERS  := $(patsubst tests/%.c,run-%,$(TEST_SOURCES))


.PHONY: all application bootloader clean inspect disasm test run-tets $(TEST_RUNNERS)

all: application bootloader disasm

application: $(APP_IMG)

bootloader: $(BOOT_BIN)

disasm: $(APP_LST) $(BOOT_LST)

$(APP_LST): $(APP_ELF)
	$(OBJDUMP) -d -S $< > $@

$(BOOT_LST): $(BOOT_ELF)
	$(OBJDUMP) -d -S $< > $@


$(APP_BUILD_DIR)/obj/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(APP_CFLAGS) -c $< -o $@

$(APP_LDS): application/linker.ld.S
	@mkdir -p $(dir $@)
	$(LDS_CPP) $(APP_INCLUDES) -MT $@ $< -o $@

$(APP_ELF): $(APP_OBJECTS) $(APP_LDS)
	$(CC) $(BASE_LDFLAGS) $(APP_OBJECTS) -T $(APP_LDS) \
	      -Wl,-Map=$(APP_BUILD_DIR)/application.map -o $@

$(APP_BIN): $(APP_ELF)
	$(OBJCOPY) -O binary $< $@

$(APP_IMG): $(APP_BIN) scripts/package_image.py common/include/memory_map.h common/include/image_format.h
	$(PYTHON) scripts/package_image.py $< $@ \
	    --magic $(call cpp_value,APP_IMAGE_MAGIC) \
	    --load-address $(call cpp_value,APP_SDRAM_BASE)


$(BOOT_BUILD_DIR)/obj/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(BOOT_CFLAGS) -c $< -o $@

$(BOOT_LDS): bootloader/linker.ld.S
	@mkdir -p $(dir $@)
	$(LDS_CPP) $(BOOT_INCLUDES) -MT $@ $< -o $@

$(BOOT_ELF): $(BOOT_OBJECTS) $(BOOT_LDS)
	$(CC) $(BASE_LDFLAGS) $(BOOT_OBJECTS) -T $(BOOT_LDS) \
	      -Wl,-Map=$(BOOT_BUILD_DIR)/bootloader.map -o $@

$(BOOT_BIN): $(BOOT_ELF)
	$(OBJCOPY) -O binary $< $@

build/tests/test_crc16:          SRC_DEPS := lib/crc/crc16.c
build/tests/test_xmodem:         SRC_DEPS := lib/crc/crc16.c lib/protocol/xmodem/xmodem.c
build/tests/test_xmodem_receive: SRC_DEPS := lib/crc/crc16.c lib/protocol/xmodem/xmodem.c
build/tests/test_xmodem_drain:   SRC_DEPS := bootloader/src/boot_xmodem_io.c

tests: $(TEST_BINARIES)

run-tests: $(TEST_RUNNERS)

$(TEST_RUNNERS): run-%: build/tests/%
	@echo "\n----"
	@echo "Running $*..."
	@./$<

build/tests/%: tests/%.c
	@mkdir -p $(@D)
	gcc $(TEST_CFLAGS) $(TEST_INCLUDES) $< $(SRC_DEPS) -o $@

clean:
	rm -rf $(APP_BUILD_DIR) $(BOOT_BUILD_DIR) $(TEST_BUILD_DIR)

# Flash bootloader.bin at 0x08000000 and application.img at 0x08020000 (APP_FLASH_BASE).
inspect: all
	$(SIZE) $(BOOT_ELF) $(APP_ELF)
	$(OBJDUMP) -h $(BOOT_ELF) $(APP_ELF)
	$(READELF) -l $(APP_ELF)

-include $(shell find $(BUILD_DIR) -name '*.d' 2>/dev/null)
