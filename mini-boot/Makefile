BOARD ?= stm32f429i-disc1


.DEFAULT_GOAL := all


BUILD  := build/$(BOARD)
TARGET := $(BUILD)/mini-boot


# Generic sources; the fragments below append their own with +=
SRCS := common/main.c
SRCS += common/console.c
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


CFLAGS   = $(CPUFLAGS) -ffreestanding -Wall -Wextra -g -Os \
		   -ffunction-sections -fdata-sections \
		   -Iinclude -Iarch/$(ARCH)/include -Iarch/$(ARCH)/mach-$(MACH)/include -Iboard/$(BOARD) \
		   -MMD -MP
LDFLAGS  = $(CPUFLAGS) -nostdlib -T $(LDSCRIPT) -L board/$(BOARD) \
		   -Wl,--gc-sections -Wl,-Map=$(TARGET).map

.PHONY: all flash disasm clean

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

flash: $(TARGET).elf
	openocd -f $(OPENOCDCFG) -c "program $< verify reset exit"
	@echo ""

disasm: $(TARGET).elf
	$(OBJDUMP) -d $<
	@echo ""

clean:
	rm -rf build

-include $(OBJS:.o=.d)