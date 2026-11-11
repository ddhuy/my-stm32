# mini-boot: project context

## Goal
Build a mini bootloader with a driver model like U-Boot / the Linux kernel
(device, driver, uclass, bind/probe). First target: STM32F429I-DISC1
(Cortex-M4F). The source tree is laid out so that more architectures, SoCs
and boards can be added. The purpose is learning.

## How we work
- I write the code myself. Claude guides one step at a time: what each file
  must contain and why, the function signatures, the traps, and a list of
  checks with expected results. Then Claude reviews what I wrote.
- Claude does not write or edit source files unless I explicitly ask
  ("help me fix ...").
- Go slowly. Theory first, minimal code, one step per session if needed.
- No bare numbers in code: field widths, shifts and offsets get names.
- `../mini-boot.old/` is an earlier flat-layout version that Claude wrote,
  complete up to printf/malloc/fault handler and tested on the board. Use it
  as a reference, not as something to copy wholesale.

## Background
I have already done bare-metal work on this board as single-file main.c
labs: vector table, linker script, startup (.data/.bss), UART printf,
SysTick/EXTI, SPI gyroscope, PendSV task switcher, 168 MHz PLL from HSE,
FMC SDRAM at 0xD0000000. Toolchain: arm-none-eabi-*, OpenOCD, Ubuntu 24.04.

## Project structure
Four levels; each fact lives at the level that owns it.

mini-boot/
├── Makefile                      make BOARD=<board>; output in build/<board>/
├── arch/arm/                     true for every ARM chip
│   ├── arch.mk                   default CROSS_COMPILE
│   ├── include/asm/              io.h (readl/writel), armv7m.h
│   ├── cpu/armv7m/               true for every Cortex-M3/M4/M7
│   │   ├── cpu.mk                adds its sources, names the linker script
│   │   └── start.S, fault.c, link.ld (sections only, INCLUDE memory.ld)
│   └── mach-stm32f4/             true for one SoC family
│       ├── mach.mk, clock.c
│       └── include/mach/         stm32f429.h (base addresses only),
│                                 clock_ids.h (CLK_*), irqs.h (NR_IRQS)
├── board/stm32f429i-disc1/       what is soldered on one board
│   ├── board.mk                  ARCH, CPU, MACH, CPUFLAGS, OpenOCD config,
│   │                             and which drivers this board uses
│   └── board.c, board.h, memory.ld (FLASH/RAM sizes)
├── drivers/core/ gpio/ serial/ spi/
├── include/                      generic APIs: types.h, init.h, clock.h,
│                                 gpio.h, serial.h, ...
├── lib/                          printf, string, malloc (no hardware access)
└── common/                       main.c, console.c

Adding a board on the same SoC = a new board/<name>/. A new SoC family = a
new arch/arm/mach-<name>/. A new architecture = a new arch/<name>/.
Planned second target to prove the split: QEMU lm3s6965evb (Cortex-M3,
different SoC and UART), not started.

## Build
- `make` (default BOARD=stm32f429i-disc1), `make flash` (OpenOCD),
  `make disasm`, `make clean` (removes all of build/).
- The top Makefile sets the generic SRCS, then includes board.mk, arch.mk,
  cpu.mk, mach.mk in that order; each fragment appends with `SRCS +=`.
- Include flags: -Iinclude -Iarch/$(ARCH)/include
  -Iarch/$(ARCH)/mach-$(MACH)/include -Iboard/$(BOARD).
- Always look at the `size` output: a successful make with text = 0 means a
  source file fell out of SRCS.

## Layering rules
- common/, lib/, drivers/ and include/ never include <mach/...> or board.h.
  Check: `grep -rn "mach/\|board.h" common drivers include lib` prints
  nothing.
- Drivers never hardcode an instance, pins or baud rate; they receive base
  address, clock rate and config from the board, and access registers via
  readl/writel(base + offset) from <asm/io.h>.
- A register layout is private to the one .c file that drives the block
  (RCC in mach-stm32f4/clock.c, GPIO in drivers/gpio/stm32_gpio.c).
  mach/stm32f429.h holds base addresses only.
- include/clock.h is the contract every SoC implements (clock_enable,
  clock_get_rate); what a clock ID means is in mach/clock_ids.h.
- A board chooses its drivers in board.mk. board.c contains no addresses and
  no register offsets; everything comes from board.h.
- u32 is `unsigned int` (not uint32_t) so it matches printf %u / %x.
- Functions return 0 or -Exxx once include/errno.h exists.

## Status (2026-11-11)
Steps 1-3 done and tested on the board: the green LED blinks.
- Step 1: start.S, link.ld, memory.ld, irqs.h. Vector table 107 words.
- Step 2: Makefile and the four .mk fragments.
- Step 3: types.h, asm/io.h, mach headers, clock.c, stm32_gpio.c, board.c,
  init.h, main.c (LED heartbeat on the 16 MHz HSI). clock_get_rate() and the
  GPIO alternate-function code (gpio_set_alternative) are already written,
  ahead of step 4.

Empty placeholders: fault.c, asm/armv7m.h, include/serial.h, include/spi.h.

## Roadmap
4. Serial console: polled USART driver (drivers/serial/stm32_serial.c behind
   include/serial.h), common/console.c (putc/puts; which UART is the console
   is one static variable), board.c wires clock + pin mux + UART. Check: a
   banner on /dev/ttyACM0 at 115200.
5. lib/string.c and lib/printf.c.
6. lib/malloc.c (bump allocator, heap after .bss in link.ld),
   include/errno.h, HardFault report in fault.c + panic().
7. Driver model: theory, then drivers/core/. Decisions to make first:
   - driver registration: linker-generated array (needs a KEEP section in
     link.ld because of --gc-sections) or a table in board.c;
   - board/driver contract: proposed include/platdata/stm32_serial.h etc.;
   - uclass device storage: fixed array or linked list.
8. Convert the GPIO and serial drivers to the driver model.
Later: second target (QEMU lm3s6965evb), 168 MHz PLL, SDRAM, SPI.
