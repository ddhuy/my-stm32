# mini-boot: project context

## Goal
Build a mini bootloader for the STM32F429I-DISC1 (Cortex-M4F) with a driver
model like U-Boot / the Linux kernel (device, driver, uclass, bind/probe).
The purpose is learning the driver model, not porting to many boards.
Teach theory first, with minimal code, step by step.

## Background
I have already done bare-metal work on this board as single-file main.c
labs: vector table, linker script, startup (.data/.bss), UART printf,
SysTick/EXTI, SPI gyroscope, PendSV task switcher, 168 MHz PLL from HSE,
FMC SDRAM at 0xD0000000. Toolchain: arm-none-eabi-*, OpenOCD, Ubuntu 24.04.

## Decided project structure
A trimmed U-Boot-like layout. The full arch/arm/cpu/armv7m + mach-stm32 +
board/st/<board> split was rejected as over-engineering for one board.

mini-boot/
├── Makefile
├── arch/                  CPU core + SoC together
│   ├── start.S            vector table, reset handler
│   ├── clock.c            RCC / PLL
│   ├── link.ld
│   └── include/           armv7m.h, stm32f429.h, io.h (readl/writel)
├── board/
│   ├── board.c            pin mux, SDRAM timings, list of devices on this board
│   └── board.h            HSE = 8 MHz, console UART, LED pins (PG13/PG14)
├── drivers/
│   ├── core/              device, driver, uclass, bind/probe
│   ├── serial/  gpio/  spi/
├── include/               generic APIs: serial.h, gpio.h, dm.h, types.h
├── lib/                   printf, string (no hardware access)
├── common/                main.c (init sequence), console.c
└── build/                 all output, created by the Makefile

## Layering rules
- Include flags: -Iinclude -Iarch/include; board.c includes "board.h".
- common/ and lib/ include only headers from include/.
- Drivers never include board.h and never hardcode an instance, pins or
  baud rate; they receive base address, clock and config from the board.
- Drivers access registers via readl/writel(base + offset), so the base
  address is a runtime parameter.
- Keep CPU facts (armv7m.h) and SoC facts (stm32f429.h) in separate headers.
- A driver's private register layout sits next to its .c file.
- board.c will hold the device list (platform data) that drivers/core
  matches to drivers; a device tree may replace it later.

## Status
Everything the driver model needs is in place and tested on the board
(`make`, `make flash`; console on /dev/ttyACM0 at 115200):
- arch/: start.S, link.ld (16K heap after .bss), clock.c (clock_enable,
  clock_get_rate; HSI 16 MHz only), fault.c (HardFault report).
- drivers/gpio/stm32_gpio.c and drivers/serial/stm32_serial.c: plain polled
  drivers behind include/gpio.h and include/serial.h, with the peripheral
  base address as the handle. Register layouts are private to each .c file;
  stm32f429.h holds base addresses only.
- common/: console.c (putc/puts, console UART in one static variable),
  panic.c, main.c (banner, printf/malloc selftest, LED heartbeat).
- lib/: printf.c, string.c, malloc.c (bump allocator, free is a no-op).
- include/errno.h: functions return 0 or -Exxx.
- u32 is `unsigned int` (not uint32_t) so it matches printf %u / %x.

Postponed on purpose, not needed for the driver model: 168 MHz PLL, SDRAM,
SPI, SysTick timer.

Temporary pieces the driver model should replace: base-address handles in
gpio.h/serial.h, the static console UART in console.c, board_led_toggle()
in init.h, selftest() in main.c.

## Next steps
1. Driver-model theory, then drivers/core/. Decisions to make first:
   - driver registration: linker-generated array (needs a KEEP section in
     link.ld because of --gc-sections) or a table in board.c;
   - board/driver contract: proposed include/platdata/stm32_serial.h etc.;
   - uclass device storage: fixed array or linked list.
2. Convert the GPIO and serial drivers to the driver model.
3. Later: PLL, SDRAM, SPI.