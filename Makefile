TARGET = main

# Default target chip. Only leave one of these un-commented.
MCU ?= STM32F429I
ifeq ($(MCU), STM32F429I)
	MCU_CLASS = VVC_F4
	MCU_FILE  = STM32F429I
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
ASFLAGS += -O0
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

AS_SRC   =  ./boot_code/$(MCU_FILE)_core.S
AS_SRC   += ./vector_tables/$(MCU_FILE)_vtable.S
C_SRC    =  ./src/global.c ./src/main.c ./src/nvic.c

INCLUDE  =  -I./
INCLUDE  += -I./device_headers

OBJS  = $(AS_SRC:.S=.o)
OBJS  += $(C_SRC:.c=.o)

.PHONY: all
all: $(TARGET).bin

%.o: %.S
	$(CC) -x assembler-with-cpp $(ASFLAGS) $< -o $@

%.o: %.c
	$(CC) -c $(CFLAGS) $(INCLUDE) $< -o $@

$(TARGET).elf: $(OBJS)
	$(CC) $^ $(LFLAGS) -o $@

$(TARGET).bin: $(TARGET).elf
	$(OC) -S -O binary $< $@
	$(OS) $<

.PHONY: clean
clean:
	rm -f $(OBJS)
	rm -f $(TARGET).elf
	rm -f $(TARGET).bin
