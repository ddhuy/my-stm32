BUILD_DIR = output

# Default target chip. Only leave one of these un-commented.
MCU ?= STM32F429I
ifeq ($(MCU), STM32F429I)
	MCU_CLASS = VVC_F4
	MCU_FILE  = STM32F429ZIT6
	ST_MCU_DEF = STM32F429xx
else ifeq ($(MCU), STM32F031K6)
	MCU_CLASS = VVC_F0
	MCU_FILE  = STM32F031K6T6
	ST_MCU_DEF = STM32F031x6
else ifeq ($(MCU), STM32L031K6)
	MCU_CLASS = VVC_L0
	MCU_FILE  = STM32L031K6T6
	ST_MCU_DEF = STM32L031xx
endif

# Define the linker script location and chip architecture.
LD_SCRIPT = $(MCU_FILE).ld
ifeq ($(MCU_CLASS), VVC_F4)
	MCU_SPEC  = cortex-m4
else ifeq ($(MCU_CLASS), VVC_F0)
	MCU_SPEC  = cortex-m0
else ifeq ($(MCU_CLASS), VVC_L0)
	MCU_SPEC  = cortex-m0plus
endif

# Toolchain definitions (ARM bare metal defaults)
TOOLCHAIN = /data/toolchain/arm-gnu-toolchain-13.3.rel1-x86_64-arm-none-eabi
#TOOLCHAIN = /data/toolchain/gcc-arm-none-eabi-10.3-2021.10
CC = $(TOOLCHAIN)/bin/arm-none-eabi-gcc
AS = $(TOOLCHAIN)/bin/arm-none-eabi-as
LD = $(TOOLCHAIN)/bin/arm-none-eabi-ld
OC = $(TOOLCHAIN)/bin/arm-none-eabi-objcopy
OD = $(TOOLCHAIN)/bin/arm-none-eabi-objdump
OS = $(TOOLCHAIN)/bin/arm-none-eabi-size

# Assembly directives.
ASFLAGS += -c
ASFLAGS += -g
ASFLAGS += -mcpu=$(MCU_SPEC)
ASFLAGS += -mthumb
ASFLAGS += -Wall
# (Set error messages to appear on a single line.)
ASFLAGS += -fmessage-length=0

# C compilation directives
CFLAGS += -mcpu=$(MCU_SPEC)
CFLAGS += -mthumb
CFLAGS += -Wall
CFLAGS += -g
# (Set error messages to appear on a single line.)
CFLAGS += -fmessage-length=0
# (Set system to ignore semi hosted junk)
CFLAGS += --specs=nosys.specs
# define the current board using
CFLAGS += -D$(ST_MCU_DEF) -D$(MCU_CLASS)

# Linker directives.
LSCRIPT = ./ld/$(LD_SCRIPT)
LFLAGS += -mcpu=$(MCU_SPEC)
LFLAGS += -mthumb
LFLAGS += -Wall
LFLAGS += --specs=nosys.specs
LFLAGS += -nostdlib
LFLAGS += -lgcc
LFLAGS += -T$(LSCRIPT)

AS_SRCS  =  ./boot_code/$(MCU_FILE)_core.S \
            ./vector_tables/$(MCU_FILE)_vt.S
AS_OBJS  = $(AS_SRCS:.S=.o)

INCLUDE  =  -I./ \
            -I./device_headers

%.o: %.S
	$(CC) -x assembler-with-cpp $(ASFLAGS) $< -o $@

%.o: %.c
	$(CC) -c $(CFLAGS) $(INCLUDE) $< -o $@


.PHONY: all
all: isr_button tft_lcd


.PHONY: clean
clean:
	rm -f $(AS_OBJS) $(TFT_LCD_OBJS) $(ISR_BUTTON_OBJS)
	rm -rf $(BUILD_DIR)/


ISR_BUTTON_SRCS  = $(wildcard apps/isr_button/*.c)
ISR_BUTTON_OBJS  = $(ISR_BUTTON_SRCS:.c=.o)

.PHONY: isr_button
isr_button: $(BUILD_DIR)/isr_button.bin
$(BUILD_DIR)/isr_button.elf: $(AS_OBJS) $(ISR_BUTTON_OBJS)
	@mkdir -p $(BUILD_DIR)/
	$(CC) $^ $(LFLAGS) -o $@
$(BUILD_DIR)/isr_button.bin: $(BUILD_DIR)/isr_button.elf
	$(OC) -S -O binary $< $@
	$(OS) $<


TFT_LCD_SRCS  = $(wildcard apps/tft_lcd/*.c)
TFT_LCD_OBJS  = $(TFT_LCD_SRCS:.c=.o)

.PHONY: tft_lcd
tft_lcd: $(BUILD_DIR)/tft_lcd.bin
$(BUILD_DIR)/tft_lcd.elf: $(AS_OBJS) $(TFT_LCD_OBJS)
	@mkdir -p $(BUILD_DIR)/
	$(CC) $^ $(LFLAGS) -o $@
$(BUILD_DIR)/tft_lcd.bin: $(BUILD_DIR)/tft_lcd.elf
	$(OC) -S -O binary $< $@
	$(OS) $<
