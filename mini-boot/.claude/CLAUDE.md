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
  ("help me fix ...", "add the comments"). A review is read-only: build,
  run the checks, report; do not fix.
- Go slowly. Theory first, minimal code, one part per message. Do not move
  to the next part until I say so.
- No bare numbers in code: field widths, shifts and offsets get names.
- I leave empty `/** */` blocks while coding and ask Claude to fill them
  when a step is finished. A comment pass changes comment lines only.
- I commit myself. Claude helps with the commit message when asked.
- board_print_info() in board.c prints the whole banner box; keep it so.
- Always review my work; don't expect me to get everything right.

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
│   ├── include/asm/              io.h (readl/writel), armv7m.h (SCB)
│   ├── cpu/armv7m/               true for every Cortex-M3/M4/M7
│   │   ├── cpu.mk                adds its sources, names the linker script
│   │   └── start.S, fault.c, link.ld (sections only, INCLUDE memory.ld)
│   └── mach-stm32f4/             true for one SoC family
│       ├── mach.mk               adds clock.c, devices.c and the drivers
│       │                         of the in-chip devices (SysTick)
│       ├── clock.c, devices.c    devices.c: DM_DEVICE rows for blocks
│       │                         every board on this SoC has
│       └── include/mach/         stm32f429.h (base addresses only),
│                                 clock_ids.h (CLK_*), irqs.h (NR_IRQS)
├── board/stm32f429i-disc1/       what is soldered on one board
│   ├── board.mk                  ARCH, CPU, MACH, CPUFLAGS, OpenOCD config,
│   │                             and which hardware drivers this board uses
│   └── board.c (hooks + device table), board.h, memory.ld (FLASH/RAM,
│                                 heap and stack sizes)
├── drivers/
│   ├── core/                     driver.c, device.c, uclass.c, root.c
│   ├── serial/                   serial-uclass.c (generic), stm32_serial.c
│   ├── gpio/                     gpio-uclass.c (generic), stm32_gpio.c
│   └── timer/                    timer-uclass.c (generic), armv7m_systick.c
├── include/                      generic APIs: types.h, errno.h, init.h,
│   │                             clock.h, gpio.h, serial.h, timer.h,
│   │                             console.h, systime.h,
│   │                             printf.h, string.h, malloc.h, panic.h
│   ├── dm/                       uclass-id.h, driver.h, device.h, uclass.h,
│   │                             root.h
│   └── platdata/                 stm32_serial.h, stm32_gpio.h,
│                                 armv7m_systick.h: what a board (or SoC)
│                                 tells a driver about one instance
├── lib/                          printf, string, malloc (no hardware access)
└── common/                       main.c, console.c, panic.c, systime.c

Adding a board on the same SoC = a new board/<name>/. A new SoC family = a
new arch/arm/mach-<name>/. A new architecture = a new arch/<name>/.
Planned second target to prove the split: QEMU lm3s6965evb (Cortex-M3,
different SoC and UART), not started.

## Build
- `make` (default BOARD=stm32f429i-disc1), `make flash` (OpenOCD),
  `make disasm`, `make clean` (removes all of build/).
- The top Makefile sets the generic SRCS (common/, lib/, drivers/core/,
  uclass files), then includes board.mk, arch.mk, cpu.mk, mach.mk in that
  order; each fragment appends with `SRCS +=`.
- Include flags: -Iinclude -Iarch/$(ARCH)/include
  -Iarch/$(ARCH)/mach-$(MACH)/include -Iboard/$(BOARD).
- Warnings: -Wall -Wextra -Wmissing-prototypes, all errors (-Werror);
  linker warnings too (-Wl,--fatal-warnings). For an experiment that
  needs a warning, drop -Werror for that one build.
  A .c file includes the header that declares what it defines, so the
  compiler compares the two. A C function called only from assembly
  (hardfault_report) gets a prototype in its own file.
- Always look at the `size` output. text = 0 means a source file fell out
  of SRCS; an unexpected drop means the compiler deleted code it proved
  unreachable (an inverted check).
- Console: /dev/ttyACM0 at 115200 (ST-LINK VCP, USART1 on PA9/PA10 AF7).
  LEDs: PG13 green (heartbeat), PG14 red (panic). Clock: 16 MHz HSI.

## Layering rules
- common/, lib/, drivers/ and include/ never include <mach/...> or board.h.
  Check: `grep -rn "mach/\|board\.h" common drivers include lib` prints
  nothing. (Escape the dot: "board.h" also matches the words "board hook".)
- Drivers never hardcode an instance, pins or baud rate; they receive base
  address, clock ID and config from the board's platform data, and access
  registers via readl/writel(base + offset) from <asm/io.h>.
- A register layout is private to the one .c file that drives the block
  (RCC in mach-stm32f4/clock.c, GPIO in drivers/gpio/stm32_gpio.c).
  mach/stm32f429.h holds base addresses only.
- include/clock.h is the contract every SoC implements (clock_enable,
  clock_get_rate); what a clock ID means is in mach/clock_ids.h. A driver
  passes the ID on as an opaque number.
- A board chooses its hardware drivers in board.mk. board.c contains no
  addresses and no register offsets; everything comes from board.h.
- u32 is `unsigned int` (not uint32_t) so it matches printf %u / %x.
- Functions return 0 (SUCCESS) or -Exxx from include/errno.h. A pointer
  result fails as NULL (`if (!ptr)`); an error code fails as non-zero
  (`if (ret)`). A yes/no function such as tstc never returns an error code.
- A header that only uses `struct x *` forward-declares it instead of
  including its header (dm/driver.h and dm/device.h must not include each
  other).
- Linker-script symbols are addresses: declare them as arrays
  (`extern const struct driver _sdm_drivers[];`), never as pointers.

## Driver model (as built)
Decisions: drivers and devices are collected by the linker (KEEP sections
.dm_drivers and .dm_devices in link.ld); the board/driver contract is
include/platdata/*.h; each uclass stores its devices in a fixed array
(UCLASS_MAX_DEVICES), to become a linked list later.

- DM_DRIVER(name) = { .name, .id, .probe, .ops, .priv_auto } in the driver's
  .c file. DM_DEVICE(name) = { .name, .driver, .plat } in board.c.
- dm_init() binds every DM_DEVICE: driver_find() by name, calloc a
  struct udevice, add it to the driver's uclass. No hardware is touched.
  A failed row is reported and skipped.
- uclass_find_device(id, index) only looks up. uclass_get_device() also
  calls device_probe(): allocate priv (priv_auto bytes), call the driver's
  probe, set `probed` last. Probe runs once per device, on first use.
  uclass_find_device_by_name() / uclass_get_device_by_name() do the same by
  name; a name is unique within its uclass.
- plat is the board's constant data in flash; priv is the driver's own data
  on the heap.
- A uclass defines an ops table (struct serial_ops in serial.h, struct
  gpio_ops in gpio.h) and generic functions (serial-uclass.c,
  gpio-uclass.c) that dispatch through dev->driver->ops and return -ENOSYS
  for a missing operation.
- GPIO: one device is one port, a pin is a number within it. Probe turns on
  the port clock. Pin multiplexing is a GPIO operation (set_alternative);
  there is no pinctrl uclass. The board, not the serial driver, sets up the
  console pins.
- Boot order in main(): dm_init(), board_init() (gets its GPIO ports by
  name: LEDs, then console pins), console_init() (gets serial device 0,
  which probes it), systime_init() (timer device 0), then the first print.
  The main loop never blocks: continuous echo, LED toggled every BLINK_MS
  by systime_elapsed_ms(). A failure before the console is up is silent;
  panic() lights the red LED if the LED port was found.
- Timer (step 9): SysTick is a CPU block present on every board of the
  SoC, so its DM_DEVICE row is in mach-stm32f4/devices.c, and mach.mk (not
  board.mk) adds its driver. Its base (SYSTICK_BASE) is in asm/armv7m.h;
  its register layout is private to drivers/timer/armv7m_systick.c. Its
  clock is CLK_HCLK (bus CLK_BUS_CORE, not an RCC bus); clock_enable()
  ignores it and clock_get_rate() gives the frequency. The timer uclass
  has one operation, get_ms (no error code; a missing table gives 0). The
  tick counter lives in priv: the first driver that uses priv. The
  handler gets no dev, so the driver keeps one static device pointer, set
  in probe before TICKINT; a second SysTick device gets -EBUSY.
- System time (common/systime.c, include/systime.h) sits on the timer
  uclass as console.c sits on the serial uclass: systime_init() takes
  timer device 0; systime_elapsed_ms(base) and systime_mdelay(ms) need no
  dev. Not named time.h: one letter from timer.h, and it would shadow the
  C library's <time.h>. Time values are u32; compare elapsed times
  (systime_elapsed_ms(start) >= timeout), never now >= start + timeout
  (tested across the 2^32 wrap in step 9: the wrong form bursts, then
  freezes). systime_elapsed_ms() returns 0 before systime_init(), so
  systime_mdelay() before systime_init() never returns.
- The order of entries in a linker section is not guaranteed; never assume
  an index means a particular device (gpioa came out as GPIO device 0).
  Use the lookup by name; the names are in board.h (LED_GPIO_NAME,
  CONSOLE_GPIO_NAME).

## Status (2026-11-18)
Steps 1-9 done, tested on the board and committed (step 9: bb0fa77,
539fa0c, a0649bc). The rename time.* -> systime.* is not committed yet.
- Steps 1-3: startup, linker script, Makefile, clock and GPIO drivers,
  LED heartbeat.
- Step 4: polled USART driver and console; banner and echo.
- Step 5: lib/string.c, lib/printf.c (%c %s %d %u %x %p %%, width, zero pad).
- Step 6: bump allocator (8-byte alignment, heap after .bss, 16K), errno.h,
  HardFault report (pc, lr, CFSR, HFSR) and panic().
- Step 7: driver model core; three devices bound (gpioa, gpiog, usart1);
  the serial driver fully converted (probe + ops); the console runs on a
  serial device.
- Step 8: lookup by name; the GPIO driver fully converted (probe + ops);
  board.c reaches the LEDs and the console pins through GPIO devices and no
  longer calls a driver with a base address.
- Step 9: timer uclass and SysTick driver (row at the SoC level,
  interrupt, ms counter in priv; 60 006 ms over 60 s); systime_elapsed_ms()
  and systime_mdelay(); the delay loop (delay(500000) was 408 ms) replaced
  by a non-blocking main loop; a pasted line echoes back whole.

Known limitations:
- free() does nothing, so a device that fails after allocation is leaked.
- "Serial device 0" is the console and "timer device 0" the system timer;
  no way yet to choose among several.
- Bind errors printed before console_init() are lost.
- gpio_get_value() can return a negative error; board_led_toggle() does not
  test for it.
- The two uclass "get" functions duplicate their probe-and-return tail.
- include/spi.h is an empty placeholder.
- A NULL dereference does not fault: address 0 aliases flash, a read
  returns the vector table, a write is dropped and sets FLASH_SR PGSERR /
  PGPERR (seen in step 9, priv_auto removed). A driver using dev->priv must
  declare .priv_auto; nothing checks it. An MPU no-access region at 0 would
  catch it.

## Roadmap
The main purpose is learning the driver model. Agreed order:
1. Timer (SysTick): done (step 9).
2. Second target on QEMU (lm3s6965evb) to prove the four-level split.
3. Clock uclass, with the 168 MHz PLL from HSE as its first real use.
4. SPI, with the L3GD20 gyroscope as a child device (parent/child).
5. Command shell, including a `dm` command.
6. Booting an application.
7. SDRAM (FMC at 0xD0000000), firm, likely with a pinctrl uclass.
Linked-list uclass storage when the fixed array gets in the way.
