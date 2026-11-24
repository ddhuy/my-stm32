BOARD ?= stm32f429i-disc1


.DEFAULT_GOAL := all


BUILD  := build/$(BOARD)
TARGET := $(BUILD)/mini-boot


# Generic sources; the fragments below append their own with +=
SRCS := common/main.c

SRCS += common/console.c
SRCS += common/dtb.S
SRCS += common/panic.c
SRCS += common/systime.c

SRCS += drivers/core/device.c
SRCS += drivers/core/driver.c
SRCS += drivers/core/read.c
SRCS += drivers/core/root.c
SRCS += drivers/core/uclass.c

SRCS += drivers/clk/clk-uclass.c
SRCS += drivers/gpio/gpio-uclass.c
SRCS += drivers/gyro/gyro-uclass.c
SRCS += drivers/serial/serial-uclass.c
SRCS += drivers/spi/spi-uclass.c
SRCS += drivers/timer/timer-uclass.c

SRCS += lib/fdt.c
SRCS += lib/malloc.c
SRCS += lib/printf.c
SRCS += lib/string.c


include board/$(BOARD)/board.mk
include arch/$(ARCH)/arch.mk
include arch/$(ARCH)/cpu/$(CPU)/cpu.mk
include arch/$(ARCH)/mach-$(MACH)/mach.mk


CC	    := $(CROSS_COMPILE)gcc
OBJCOPY := $(CROSS_COMPILE)objcopy
OBJDUMP := $(CROSS_COMPILE)objdump
SIZE	:= $(CROSS_COMPILE)size


OBJS = $(patsubst %,$(BUILD)/%.o,$(basename $(SRCS)))


CFLAGS   = $(CPUFLAGS) -ffreestanding \
		   -Wall -Werror -Wextra -Wmissing-prototypes \
		   -g -Os \
		   -ffunction-sections -fdata-sections \
		   -Iinclude -Iarch/$(ARCH)/include \
		   -Iarch/$(ARCH)/mach-$(MACH)/include -Iboard/$(BOARD) \
		   -MMD -MP
LDFLAGS  = $(CPUFLAGS) -nostdlib -T $(LDSCRIPT) -L board/$(BOARD) \
		   -Wl,--gc-sections -Wl,--fatal-warnings -Wl,-Map=$(TARGET).map

# Device tree: board/<board>/<board>.dts, run through the C preprocessor
# (dt-bindings headers, the cpu and SoC .dtsi files), then dtc. -undef: no
# predefined macro may rewrite a word of the .dts. dtc -@ writes the
# /__symbols__ node (label -> path): devices are named after their label.
DTS := board/$(BOARD)/$(BOARD).dts
DTB := $(BUILD)/$(BOARD).dtb
DTC := dtc
DTS_CPPFLAGS = -E -x assembler-with-cpp -nostdinc -undef -D__DTS__ \
		   -Iinclude -Iarch/$(ARCH)/cpu/$(CPU) -Iarch/$(ARCH)/mach-$(MACH)

# make check: every board in board/, so a new board joins automatically
BOARDS := $(notdir $(wildcard board/*))

# Boot test: the line main() prints just before the main loop (1000 or 1001)
QEMU_BOOT_SECS := 5
QEMU_BOOT_OK   := systime_mdelay(1000): 100


.PHONY: all flash disasm clean run debug check boot-test

all: $(TARGET).bin
	$(SIZE) $(TARGET).elf

$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@
	@echo ""

$(TARGET).elf: $(OBJS) $(LDSCRIPT) board/$(BOARD)/memory.ld
	$(CC) $(LDFLAGS) $(OBJS) -lgcc -o $@
	@echo ""

$(BUILD)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@
	@echo ""

$(BUILD)/%.o: %.S
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@
	@echo ""

$(DTB): $(DTS)
	@mkdir -p $(dir $@)
	$(CC) $(DTS_CPPFLAGS) -MMD -MP -MT $@ -MF $@.d $< -o $@.tmp
	$(DTC) -@ -I dts -O dtb -o $@ $@.tmp
	@echo ""

# .incbin is invisible to -MMD: without this line an edited .dts gives a
# new .dtb and an old image
$(BUILD)/common/dtb.o: $(DTB)
$(BUILD)/common/dtb.o: CFLAGS += -DDTB_FILE='"$(DTB)"'

flash: $(TARGET).elf
	openocd -f $(OPENOCDCFG) -c "program $< verify reset exit"
	@echo ""

disasm: $(TARGET).elf
	$(OBJDUMP) -d $<
	@echo ""

run: $(TARGET).elf
	qemu-system-arm -M $(QEMU_MACHINE) -nographic -kernel $<

debug: $(TARGET).elf
	qemu-system-arm -M $(QEMU_MACHINE) -nographic -kernel $< -s -S

# Everything that must stay true for every board. Not tied to one BOARD:
# it runs make again for each one, and stops at the first failure.
check:
	@echo "check: layering"
	@! grep -rn "mach/\|board\.h" common drivers include lib
	@for b in $(BOARDS); do \
		$(MAKE) --no-print-directory BOARD=$$b all boot-test || exit 1; \
	done
	@echo "check: all boards passed"

# Boot this board in QEMU and look for the line printed just before the
# main loop: DM, board, console and timer all came up. timeout's own exit
# status (124) is ignored; grep's decides. stdin closed so that QEMU does
# not take over the terminal.
boot-test: $(TARGET).elf
ifeq ($(QEMU_MACHINE),)
	@echo "boot-test: $(BOARD) has no QEMU machine, skipped"
else
	@timeout $(QEMU_BOOT_SECS) qemu-system-arm -M $(QEMU_MACHINE) \
		-nographic -kernel $< < /dev/null 2>&1 | grep -q '$(QEMU_BOOT_OK)' \
		&& echo "boot-test: $(BOARD) boots in QEMU" \
		|| { echo "boot-test: $(BOARD) did not reach the main loop"; exit 1; }
endif

clean:
	rm -rf build

-include $(OBJS:.o=.d)
-include $(DTB).d
