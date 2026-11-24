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
│   │   ├── armv7-m.dtsi          systick node (every Cortex-M has one)
│   │   └── start.S, fault.c, link.ld (sections only, INCLUDE memory.ld)
│   ├── mach-stm32f4/             true for one SoC family
│   │   ├── mach.mk               adds the drivers of the in-chip devices
│   │   │                         (SysTick, RCC)
│   │   ├── stm32f429.dtsi        the SoC's nodes (rcc, gpioa-g, usart1,
│   │   │                         spi5; peripherals disabled), &systick
│   │   └── include/mach/         stm32f429.h (base addresses only),
│   │                             irqs.h (NR_IRQS)
│   └── mach-lm3s/                the same files for TI Stellaris LM3S:
│                                 lm3s6965.h, irqs.h, lm3s6965.dtsi,
│                                 mach.mk
├── board/stm32f429i-disc1/       what is soldered on one board
│   ├── board.mk                  ARCH, CPU, MACH, CPUFLAGS, OpenOCD config,
│   │                             and which hardware drivers this board uses
│   ├── stm32f429i-disc1.dts      the board's device tree: includes the
│   │                             SoC .dtsi, enables what it uses, crystal,
│   │                             speeds, the gyro under spi5
│   └── board.c (hooks), board.h (names, pins), memory.ld (FLASH/RAM,
│                                 heap and stack sizes)
├── board/lm3s6965evb/            QEMU machine lm3s6965evb: the same five
│                                 files; board.mk sets QEMU_MACHINE
├── drivers/
│   ├── core/                     driver.c, device.c, uclass.c, root.c,
│   │                             read.c (dev_read_*: a device's node)
│   ├── clk/                      clk-uclass.c (generic), lm3s_sysctl.c,
│   │                             stm32f4_rcc.c
│   ├── serial/                   serial-uclass.c (generic), stm32_serial.c,
│   │                             pl011_serial.c
│   ├── gpio/                     gpio-uclass.c (generic), stm32_gpio.c
│   ├── gyro/                     gyro-uclass.c (generic), l3gd20.c
│   ├── spi/                      spi-uclass.c (generic), stm32_spi.c
│   └── timer/                    timer-uclass.c (generic), armv7m_systick.c
├── include/                      generic APIs: types.h, errno.h, init.h,
│   │                             clk.h, gpio.h, gyro.h, serial.h, spi.h,
│   │                             timer.h,
│   │                             console.h, systime.h, fdt.h,
│   │                             printf.h, string.h, malloc.h, panic.h
│   ├── dm/                       uclass-id.h, driver.h, device.h, uclass.h,
│   │                             root.h, read.h
│   ├── dt-bindings/              clock/stm32f4-rcc.h, clock/lm3s-sysctl.h,
│   │                             gpio/gpio.h: defines only, shared by C
│   │                             and the .dts (clock IDs, GPIO flags)
│   └── platdata/                 stm32_serial.h, stm32_gpio.h,
│                                 armv7m_systick.h, pl011_serial.h,
│                                 lm3s_sysctl.h, stm32f4_rcc.h,
│                                 stm32_spi.h: what a
│                                 driver needs about one instance (filled
│                                 by its of_to_plat); a clock provider's
│                                 header also holds its device name
├── lib/                          printf, string, malloc, fdt (the blob
│                                 reader; no hardware access)
└── common/                       main.c, console.c, panic.c, systime.c,
                                  dtb.S (the blob, .incbin into flash)

Adding a board on the same SoC = a new board/<name>/. A new SoC family = a
new arch/arm/mach-<name>/. A new architecture = a new arch/<name>/.
Second target, which proved the split: QEMU lm3s6965evb (Cortex-M3,
different SoC and UART; roadmap item 2, done). QEMU 10.2 has no STM32F429
machine; the closest is the STM32F405 (netduinoplus2, olimex-stm32-h405),
a possible third board on mach-stm32f4 (own memory.ld: 128K RAM).

Every mach-<soc>/ provides: include/mach/<soc>.h (base addresses),
irqs.h (NR_IRQS, used by start.S for the vector table length),
<soc>.dtsi (the SoC's nodes, and SysTick's clock) and mach.mk (which
also adds the SoC's clock provider driver). No clock code: that is a
driver in drivers/clk/ since item 3.

## Build
- `make` (default BOARD=stm32f429i-disc1), `make flash` (OpenOCD),
  `make disasm`, `make clean` (removes all of build/).
- `make check`: the layering grep, then for every board in board/ (by
  directory name, so a new board joins by itself) `all` and
  `boot-test`. boot-test runs QEMU for QEMU_BOOT_SECS (5) and greps for
  QEMU_BOOT_OK ("systime_mdelay(1000): 100", printed just before the main
  loop); a board without QEMU_MACHINE is skipped (the STM32). Stops at the
  first failure. Run it after every change to generic code. Tested to
  fail on a layering break, a silent boot (baud 0) and a compile error.
- Pass BOARD= on every command for the other target:
  `make BOARD=lm3s6965evb run` (QEMU, -nographic) and
  `make BOARD=lm3s6965evb debug` (adds -s -S: gdb server on :1234, CPU
  held at reset; then `arm-none-eabi-gdb build/lm3s6965evb/mini-boot.elf`,
  `target remote :1234`). Quit QEMU with Ctrl-A then X (Ctrl-C goes to
  the guest); in gdb, Ctrl-C only pauses the target, `monitor quit` ends
  QEMU. A stale QEMU keeps port 1234 busy: `pkill qemu-system-arm`.
- Device tree build (item 5e): board/<board>/<board>.dts is run through
  cpp (DTS_CPPFLAGS: -x assembler-with-cpp -nostdinc -undef; -I for
  include/, cpu/<cpu>/ and mach-<mach>/, so it can #include the .dtsi
  files and the dt-bindings headers), the result in
  build/<board>/<board>.dtb.tmp; then dtc -@ makes the .dtb (merges the
  &label overrides, evaluates the expressions, numbers the phandles,
  writes /__symbols__). common/dtb.S .incbin's it into .rodata.dtb
  (8-byte aligned, _dtb_start / _dtb_end); link.ld's *(.rodata*) puts
  it in FLASH; make flash writes it with the rest. It is read in place,
  never copied. dtb.o depends on the .dtb by an explicit rule (-MMD
  cannot see .incbin); the .dtb on its .dts, .dtsi and headers
  (-include $(DTB).d). Needs dtc on the host (1.7.2 here; fdtdump too).
  Inspect: the .tmp file (after cpp), dtc -I dtb -O dts <board>.dtb
  (what is flashed), fdtdump (header, tokens). A changed Makefile flag
  (as -@ was) rebuilds nothing: make clean.
- --gc-sections drops every function nothing calls: it is then not in the
  image and gdb cannot `call` it (clock_enable() on lm3s in step 2 Part A,
  before any driver used it). The map file lists it under "Discarded input
  sections".
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
- STM32F429I-DISC1: console /dev/ttyACM0 at 115200 (ST-LINK VCP, USART1
  on PA9/PA10 AF7). LEDs: PG13 green (heartbeat), PG14 red (panic).
  Gyroscope on SPI5 (SCK PF7, MISO PF8, MOSI PF9, AF5; CS PC1, a GPIO):
  this board has the I3G4250D (WHO_AM_I 0xD3), older ones the L3GD20
  (0xD4), same registers. SPI mode 3, at most 10 MHz: 84 MHz / 16.
  SCK and MOSI need fast edges (GYRO_SPI_PIN_SPEED, OSPEEDR 10; GPIOF
  OSPEEDR 0x00088000): at the reset speed bit 0 of every byte read was
  unreliable. Gyro at rest: x ~100, y ~400, z ~450 mdps (offset),
  noise +-300; fastest seen 195 dps. The sign of z for a turn seen from
  above was not measured (skipped in Part D).
  Clock: 168 MHz from the PLL (8 MHz HSE crystal), set by the rcc
  device's probe (drivers/clk/stm32f4_rcc.c): AHB 168, APB1 42 (/4),
  APB2 84 (/2, the console UART: BRR 729). get_rate computes each bus
  from CFGR and PLLCFGR. A bad clock setup cannot brick the board:
  reset returns to HSI (if OpenOCD cannot connect, hold reset, or
  -c "reset_config connect_assert_srst").
- lm3s6965evb (QEMU 10.2.1): flash 256K at 0x0, RAM 64K at 0x20000000,
  64 NVIC lines. UART0 is a PL011 at 0x4000C000. Clock: 50 MHz from the
  PLL (8 MHz crystal, SYSDIV 3), set by the sysctl device's probe
  (drivers/clk/lm3s_sysctl.c); its get_rate computes it from RCC.
  QEMU's model:
  SysClk = 200 MHz / (SYSDIV + 1) whatever BYPASS / USESYSDIV say (12.5
  MHz at reset, where the real chip would run on the raw oscillator), so
  the only state both agree on is PLL on, not bypassed, through SYSDIV;
  get_rate returns 0 for anything else. SysTick counts only the
  CPU clock and is the only clock-rate witness in QEMU (the UART ignores
  the divisor); a wrong rate still prints mdelay(1000): 1000 ms, so check
  it against an outside clock (host time across mdelay(1000): 1.000 s).
  RCGC gating is stored, not enforced, so a missing clk_enable() is
  invisible in QEMU (check RCGC in gdb).
  QEMU's PL011 sends and receives even with UARTEN/TXE/RXE clear (it
  only logs "PL011 data written to disabled UART" with -d guest_errors)
  and ignores the baud divisor: working output proves the DM wiring, not
  the register code; check IBRD/FBRD/LCR_H/CR in gdb. No GPIO and no LED
  on this target.

## Layering rules
- common/, lib/, drivers/ and include/ never include <mach/...> or board.h.
  Check: `grep -rn "mach/\|board\.h" common drivers include lib` prints
  nothing. (Escape the dot: "board.h" also matches the words "board hook".)
- Drivers never hardcode an instance, pins or baud rate; they receive base
  address, clock ID and config in their plat, which their of_to_plat
  fills from the device tree, and access registers via
  readl/writel(base + offset) from <asm/io.h>.
- A register layout is private to the one .c file that drives the block
  (RCC in drivers/clk/stm32f4_rcc.c, System Control in
  drivers/clk/lm3s_sysctl.c, GPIO in drivers/gpio/stm32_gpio.c).
  mach/stm32f429.h holds base addresses only. One exception: the RCC
  driver also sets FLASH_ACR LATENCY and PWR_CR VOS (fields that exist
  only because of the clock; neither block has a driver), with their
  bases in its plat (flash_base, pwr_base). U-Boot does the same.
- Clocks come from a clock provider device (include/clk.h); a driver
  passes the clock ID on as an opaque number. What an ID means is in the
  provider's dt-bindings header (dt-bindings/clock/lm3s-sysctl.h,
  dt-bindings/clock/stm32f4-rcc.h, included by its platdata header),
  prefixed with the provider (LM3S_CLK_*, STM32F4_CLK_*) because a board
  may include several providers' headers. SysTick's clock is set by the
  SoC's .dtsi (&systick { clocks = <&rcc STM32F4_CLK_HCLK>; }).
- A dt-bindings header holds defines only: the .dts includes it too, and
  anything else would end up in the .dts. An expression in a cell must be
  in parentheses (STM32F4_CLK_ID() is), or dtc refuses it.
- A board chooses its hardware drivers in board.mk and describes its
  devices in its .dts. board.c contains no addresses and no register
  offsets; board.h holds only device names (each a label in the .dts)
  and pins. The .dts writes addresses as numbers, as in every Linux .dts
  (the unit address in a node's name must match its reg); IDs and flags
  come from the dt-bindings headers. /dts-v1/; only in the board's .dts,
  never in a .dtsi.
- u32 is `unsigned int` (not uint32_t) so it matches printf %u / %x.
- Functions return 0 (SUCCESS) or -Exxx from include/errno.h. A pointer
  result fails as NULL (`if (!ptr)`); an error code fails as non-zero
  (`if (ret)`). A yes/no function such as tstc never returns an error code.
- A header that only uses `struct x *` forward-declares it instead of
  including its header (dm/driver.h and dm/device.h must not include each
  other).
- Linker-script symbols are addresses: declare them as arrays
  (`extern const struct driver _sdm_drivers[];`), never as pointers.
- Header guards: MINIBOOT_ + level + owner + file name, so a guard names
  exactly one file in the tree (MINIBOOT_MACH_LM3S_CLOCK_IDS_H,
  MINIBOOT_BOARD_LM3S6965EVB_H). Generic headers in include/ have one copy
  only: MINIBOOT_<FILE>_H.

## Driver model (as built)
Decisions: the devices come only from the board's device tree (item
5e; the C table was removed after it); drivers and uclass drivers are
collected by the linker (KEEP sections .dm_drivers and .dm_uclasses in
link.ld); a driver's plat struct (include/platdata/) is what its
of_to_plat fills; each uclass stores its devices in a fixed
array (UCLASS_MAX_DEVICES), to become a linked list later.

- DM_DRIVER(name) = { .name, .id, .of_match, .of_to_plat, .probe, .ops,
  .plat_auto, .priv_auto } in the driver's .c file; .name is only
  printed (no lookup by driver name since the C table went).
- dm_init() binds the device tree (see Device tree below): every
  enabled node with a driver, calloc a struct udevice, add it to the
  driver's uclass. No hardware is touched. A failed node is reported and
  skipped. Returns uclass_device_count().
- uclass_find_device(id, index) only looks up. uclass_get_device() also
  calls device_probe(): allocate priv (priv_auto bytes), call the driver's
  probe, set `probed` last. Probe runs once per device, on first use.
  uclass_find_device_by_name() / uclass_get_device_by_name() do the same by
  name. Device names are unique across the system (DT labels are; two
  identical sensors need gyro0 / gyro1). device_find_by_node() searches
  every uclass for the device of a node (phandle resolution).
- plat: plat_auto bytes on the heap, filled by of_to_plat; priv is the
  driver's own data on the heap.
- A uclass defines an ops table (struct serial_ops in serial.h, struct
  gpio_ops in gpio.h) and generic functions (serial-uclass.c,
  gpio-uclass.c) that dispatch through dev->driver->ops and return -ENOSYS
  for a missing operation.
- GPIO: one device is one port, a pin is a number within it. Probe turns on
  the port clock. Pin multiplexing is a GPIO operation (set_alternative),
  and so is the output edge speed (set_speed, enum gpio_speed LOW ..
  VERY_HIGH, mapped one by one onto OSPEEDR in stm32_gpio.c; added in
  item 4 Part C); there is no pinctrl uclass. The board, not the
  peripheral's driver, sets up its pins (console, SPI) and chooses their
  speed: a fast signal on a slow pin is a board bug.
- Boot order in main(): dm_init(), board_init() (gets its GPIO ports by
  name: LEDs, then console pins), console_init() (gets serial device 0,
  which probes it), systime_init() (timer device 0), then the first print.
  The main loop never blocks: continuous echo, LED toggled every BLINK_MS
  by systime_elapsed_ms(), and (if gyro device 0 exists; not on lm3s)
  one "gyro: x= y= z= mdps" line every GYRO_PRINT_MS (1000), a read error
  printed once. Printing a line blocks about 3 ms, so a pasted line can
  lose characters (the USART holds one byte); a typed one is fine. The
  echo turns Enter ('\r') into '\n' (input side, like termios ICRNL);
  putc() turns '\n' into "\r\n" (output, like ONLCR); putc() must not
  translate '\r' ("\r\n" strings, progress lines, binary). A line reader
  would take this over (the shell is out of scope). A failure before
  the console is up is silent; panic() lights the red LED if the LED port
  was found. On lm3s, board_init() does nothing: the clock provider is
  probed from inside the UART's probe.
- Timer (step 9): SysTick is a CPU block present on every board of the
  SoC, so its node is in cpu/armv7m/armv7-m.dtsi, its clock in each
  SoC's .dtsi (until item 5e: a DM_DEVICE row in mach-<soc>/devices.c),
  and mach.mk (not board.mk) adds its driver. Its base (reg; also
  SYSTICK_BASE in asm/armv7m.h, for code outside the driver model) is
  0xe000e010;
  its register layout is private to drivers/timer/armv7m_systick.c. Its
  clock is the SoC's CPU clock: STM32F4_CLK_HCLK on stm32f4,
  LM3S_CLK_SYSCLK on lm3s (both in the core group, not a gate register);
  enable does nothing for it and get_rate gives the frequency. Since
  item 3 Part C SysTick is a clock consumer like the others (clk_dev
  in its plat; the -EBUSY test stays before the lookup). The timer
  uclass
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
- Decided in step 2 (Part D):
  - The SysTick row stays per SoC (mach-<soc>/devices.c, driver line in
    mach.mk; since item 5e the node is per CPU and only &systick's
    clocks is per SoC), though both copies differ only in the clock ID: which
    clock feeds SysTick is a SoC fact (some SoCs feed a divided
    reference), and moving it to cpu/armv7m would need every mach to
    export one agreed clock name. Linux does the same (SysTick in each
    SoC's device tree). The logic is shared; only ~20 lines of data are
    duplicated.
  - One platdata struct per driver (stm32_serial_plat, pl011_serial_plat)
    even with identical fields: each is one driver's contract with its
    boards and they will diverge (flow control, FIFOs, oversampling);
    U-Boot does the same.
  - Clock setup is asymmetric (lm3s programs its PLL from board_init(),
    the STM32 runs on HSI): left for the clock uclass (roadmap item 3).
- Clock uclass (item 3, decided in its theory): a provider (UCLASS_CLK:
  lm3s_sysctl, stm32f4_rcc) owns the registers, does its setup
  (PLL) in probe and implements struct clk_ops { enable, get_rate }. A
  consumer's platdata names the provider (clk_dev) and the clock
  (clk_id); its probe fills a local struct clk { dev, id } with
  clk_get_by_name(), then clk_enable() / clk_get_rate() (no priv: clocks
  are used only in probe). clk_get_by_name() uses the "get" lookup, so a
  consumer's probe probes its provider first, on demand (seen in gdb:
  lm3s_sysctl_probe under pl011_serial_probe under console_init); probe
  nests, and an error comes back up the chain. The provider's row is in
  board.c (crystal and speed are board facts); its name is a define in
  its platdata header (LM3S_SYSCTL_NAME, STM32F4_RCC_NAME), used by the
  row and every consumer. The device name is an instance name ("rcc",
  "sysctl", like "uart0"), the driver name is the code's ("stm32f4_rcc",
  "lm3s_sysctl"): kept different so that confusing the two fails. A
  provider must never call the clk API on itself: probed is set last,
  so the lookup would probe it again and recurse until the stack
  overflows. A provider without a probe
  is simply marked probed on first use. Core clocks
  (the CPU clock) have no gate: enable returns 0 for them. On the STM32
  the first consumer is the LED port, so an RCC lookup failure (a
  missing row, a misspelt clk_dev) fails board_init() with no LED left
  to show it: completely silent; the same mistake in the usart1 row
  still lights the red LED. Neither builds wrong, and the STM32 has no
  QEMU boot test: only flashing finds them. A wrong clk_dev in the
  SysTick row fails after console_init() but before board_print_info():
  the only output is "PANIC: No Timer!" (tested on the STM32), the first
  clock failure that explains itself. Where a mistake shows depends on
  where its device is first probed in the boot order.
- SPI (item 4): a bus driver (stm32_spi, device spi5, UCLASS_SPI)
  implements struct spi_ops { xfer(bus, slave, dout, din, len, flags) }
  from include/spi.h. struct spi_slave_plat { cs, mode, max_hz } is
  defined by the uclass, not a driver: it describes one slave as the
  board writes it; it is the slave device's parent_plat. Since item 5a
  Part C only spi_child_pre_probe() reads it, and the bus gets struct
  spi_slave (the run-time copy, parent_priv). A slave's driver
  calls spi_xfer(slave, ...) (spi-uclass.c), which takes the bus from
  slave->parent and refuses a parent that is not in UCLASS_SPI (-EINVAL;
  the core accepts any parent, so this is the only check) or no parent
  (-ENODEV). Mode and speed
  are per slave, so the bus programs CR1 at SPI_XFER_BEGIN (SPE clear,
  then set; the divider from the PCLK rate kept in priv) and pulls CS
  low; SPI_XFER_END waits for BSY, then raises CS (also on an error).
  A register read is two calls: address with BEGIN, data with END. DR
  is read after every byte, wanted or not. Chip select is a GPIO owned
  by the bus (cs_gpio_dev / cs_pin in its plat, like U-Boot cs-gpios),
  set high in probe before anything else; one CS (cs 0) only. SCK, MISO
  and MOSI are set up by the board (spi_pins_setup() in board_init()).
  The SPI_CPOL / SPI_CPHA bits are translated, not copied, though they
  match CR1 on the STM32.
- Parent and child (item 4 Part B; HISTORY: removed with the C table
  after item 5e, where a parent is the node a device is nested in, set
  at bind; the design is mine, chosen over the guide's "bind children in
  pass 2"): a row names its parent by string
  (.parent) and gives .parent_plat, whose type the parent's uclass
  defines. dm_init() pass 1 binds every row (a row naming a parent
  starts as an orphan); pass 2, when every device exists, calls
  device_set_parent() for each child: the only place a parent is linked.
  It refuses a cycle (-EINVAL: walking up from the parent reaches the
  child; only the closing link can make one, so the check is complete at
  any depth) and clears the orphan flag. So row order never matters, at
  any depth (seen: the gyro's udevice was allocated before spi5's).
  device_probe() refuses an orphan (-ENODEV) before anything else, then
  probes the parent first, then allocates priv: no child driver runs
  without its parent, and a failed parent costs the child no heap. In
  gdb the bus probe sits under device_probe(spi5) under
  device_probe(gyro), with no l3gd20_probe frame (in item 3 the provider
  was probed inside the consumer's probe). The device list shows each
  parent: "-" top level, "(missing)" an orphan. Tested on the board:
  misspelt parent: "(missing)", -19, spi5 never probed (9 devices still
  bound); parent "gpioc": linked, gpioc probed by the gyro's probe,
  l3gd20_probe runs, spi_xfer() -22; self-parent: cycle refused,
  "(missing)", -19, no stack overflow; rows swapped: still ready.
  Decided against (for now): a parent_id in struct driver for the core to
  check the parent's uclass; a list of children (needed by dm tree and
  device removal); a root device (parent NULL means top level).
- Gyroscope (item 4 Parts B-C): UCLASS_GYRO, struct gyro_ops { read }
  in gyro.h; struct gyro_data { x, y, z } in mdps: the unit is part of
  the uclass contract, the axes are the sensor's. l3gd20.c (L3GD20 and
  I3G4250D) is an SPI child with no plat and no priv; registers through
  l3gd20_read() (address with BEGIN, data with END; MS bit for more than
  one byte) and l3gd20_write() (one BEGIN | END transfer). Probe: write
  CTRL_REG4 whole first (BDU, BLE 0, +-250 dps, SIM 0 = 4-wire), because
  a sensor left in 3-wire mode answers no read (all 0xFF) but still
  takes writes and survives an MCU reset; then WHO_AM_I (-ENODEV); check
  BDU/BLE/FS (not SIM: a 3-wire sensor could not answer); CTRL_REG1 =
  PD | X | Y | Z, read back (-EIO). Read: six bytes from OUT_X_L, each
  axis (s16) low | high << 8, then raw * 875 / 100 (8.75 mdps/digit).
- Debugging lesson (item 4 Part C): CTRL_REG4 written 0x80 read back
  0x81, CTRL_REG1 0x0F read back 0x0E; only bit 0 (the last bit of each
  byte) was wrong, intermittently, and a pause in gdb "fixed" it. Cause:
  SCK / MOSI at the reset output speed (low) at 5.25 MHz. A
  read-modify-write then wrote the misread bit back and really set SIM
  (3-wire), so every read gave 0xFF; Part B had passed only because
  0xD3 and 0x0F end in 1. Rules: a read-back that only ever checks odd
  values hides a stuck bit 0; check the pins' speed before blaming the
  registers; never write back a bit you have not proved you read
  correctly.
- Uclass driver (item 5a Part A): struct uclass_driver { name, id } in
  dm/uclass.h, declared with UCLASS_DRIVER(gyro) in the uclass's own
  file (one row for each of the 6 ids; none for UCLASS_INVALID). The id
  is the key, the name is for printing. The run-time struct uclass
  (private to uclass.c, table uclasses[]) holds uc_drv and the devices:
  uclass_driver / uclass mirror driver / udevice. uclass_add_device()
  looks uc_drv up (uclass_driver_find(), linear by id) for the uclass's
  first device; none: the bind fails with -EPFNOSUPPORT (as in U-Boot),
  printed before the console, so it shows only in the bound count. The
  device list prints "UClass: gyro (6)", "(none)" without a row. Seen
  on the board: z at rest -250 mdps after +450 in item 4, so the gyro
  offset must be measured at boot, not written into the code.
- Uclass hooks and data (item 5a Part B): struct uclass_driver adds
  pre_probe, post_probe and per_device_auto; struct udevice adds
  uclass_priv. device_probe(): (of_to_plat since 5e), parent, priv,
  uclass_priv (same
  keep-for-retry rule), pre_probe, the driver's probe, post_probe,
  probed = true; any error stops it and the device stays unprobed (a
  retry runs everything again, the driver's probe too). The core
  reaches the uclass driver through device_get_uclass_driver(dev)
  (uclass.c; never NULL for a bound device). Hooks run with probed
  still false: they use dev, never a lookup of their own device. The
  device list shows UcPriv. First user, the gyro uclass:
  struct gyro_uc_priv { offset } private to gyro-uclass.c (only the
  uclass reads it); pre_probe refuses a driver without read (-ENOSYS)
  before it touches the sensor; post_probe calls gyro_calibrate():
  skip GYRO_CAL_SKIP (5) samples, average GYRO_CAL_SAMPLES (32), one
  every GYRO_CAL_INTERVAL_MS (10, about the reset output rate), from
  ops->read directly (through gyro_read() a second calibration would
  average the leftover and lose the offset), s32 sum divided once, the
  offset stored only after every read worked. gyro_read() subtracts
  it. l3gd20.c unchanged: calibration is the uclass's, not a driver
  op (a calibrate op was tried and removed; it was also called without
  a NULL check). Needs systime (the gyro is first probed in
  board_print_info(), after systime_init()) and a still board.
- Per-child data (item 5a Part C): struct uclass_driver adds
  child_pre_probe and per_child_auto; struct udevice adds parent_priv,
  the run-time half of parent_plat. Both are taken from the PARENT's
  uclass driver (device_get_uclass_driver(dev->parent), only if there
  is a parent), never the device's own. device_probe() order: orphan,
  parent, priv, uclass_priv, parent_priv, own pre_probe, parent's
  child_pre_probe(child), driver probe, own post_probe, probed (U-Boot's
  order). First user, the SPI uclass: per_child_auto = struct spi_slave
  { cs, mode, max_hz } (spi.h); spi_child_pre_probe() refuses a missing
  plat, mode bits outside SPI_MODE_MASK, or max_hz 0 (-EINVAL, before
  the slave's driver runs: before, a bad row failed at the first
  transfer inside l3gd20_probe()), then copies the plat (a copy, the
  live state; U-Boot's spi_slave can change speed at run time).
  spi_xfer() checks the parent's uclass first, then parent_priv: a
  slave under a non-SPI parent never ran the hook (parent_priv NULL).
  The CS range stays in stm32_spi_xfer() (a driver fact); CR1 is still
  computed at each BEGIN. Decided: per-child data at the uclass level
  only; a driver-level per_child_auto / child_pre_probe (U-Boot: the
  parent driver's size wins; Linux: setup() and controller_state) would
  let stm32_spi compute CR1 once per slave, deferred.
- A uclass hook can only see devices in its uclass or children of them,
  so child_pre_probe cannot refuse a wrong parent (a gyro under gpioc
  is invisible to the SPI uclass); that needs the child's side to name
  the parent kind it requires (the reverted parent_id). U-Boot relies
  on the device tree's structure instead.
- Device tree (item 5e; 5b's matching folded in): three levels of
  source like the C tree: cpu/armv7m/armv7-m.dtsi (systick),
  mach-<soc>/<soc>.dtsi (the SoC's blocks, peripherals status
  "disabled"), board/<board>/<board>.dts (#includes the SoC's, model,
  fixed-clock crystal, &label overrides: status "okay", speeds, the
  gyro node inside &spi5). Reader: lib/fdt.c, read-only, in place, no
  allocation (one bounded token walker, fdt_next_tag(); numbers read
  byte by byte big-endian, fdt32_ld(); nodes and properties are offsets
  into the structure block, root 0; first_child / next_sibling /
  getprop / read_u32 / first_prop / next_prop / node_by_phandle /
  path_offset / get_label / print, the last identical to dtc -O dts).
  Driver-model side:
  - Matching: struct udevice_id { compatible, data } tables (of_match);
    a node's compatible strings are tried in order (most specific
    first), driver_find_by_compatible(); the entry's data becomes
    dev->driver_data (l3gd20: a struct l3gd20_chip, probe checks
    WHO_AM_I against it: one driver, a table of chips).
  - device_bind_node(parent, node): status "okay"/"ok"/absent, a driver
    that matches, else -ENODEV (skipped quietly: clk-hse, a fixed-clock,
    has no driver). Named after its label (/__symbols__, dtc -@), else
    its node name: board.h's names stayed (U-Boot names devices by node
    name). Then: udevice, plat_auto,
    parent_plat (the parent's uclass per_child_plat_auto), uclass add,
    own post_bind, parent's uclass child_post_bind. dev->node
    (DEV_NODE_NONE for a C row).
  - of_to_plat runs first in device_probe() (before the parent's probe),
    on every attempt (idempotent). Drivers read their node with
    dm/read.h: dm_fdt(), dev_read_prop / u32 / bool, dev_read_addr_index
    (reg; cell counts from the parent node, 1-cell addresses only),
    dev_read_phandle_with_args (list walked from the start; argument
    count from the target's #clock-cells / #gpio-cells).
  - A phandle becomes the provider's device NAME (clk_read_by_index() in
    clk-uclass.c, gpio_read_by_name() in gpio-uclass.c, via
    device_find_by_node()), so every plat struct, probe and op stayed as
    with the C table and clk_get_by_name() is still the lookup. A
    fixed-clock has no device: the RCC / sysctl of_to_plat read its
    clock-frequency through the phandle directly. RCC: three reg entries
    (reg-names rcc, flash, pwr; read by index), clock-frequency = the
    SYSCLK to run.
  - Children: the SPI uclass's post_bind is dm_scan_fdt_dev() (binds the
    child nodes; post_bind's first user); spi_child_post_bind() fills
    spi_slave_plat from reg (CS), spi-max-frequency, spi-cpol, spi-cpha;
    spi_child_pre_probe() then checks and copies it as before. The bus's
    cs-gpios = <&gpioc 1 GPIO_ACTIVE_LOW> (anything but active low
    -EINVAL). A child has its parent at bind: no second pass, no
    orphans, and no cycle can exist (nesting is a tree).
  - Uclass device order is now the tree's (gpioa, gpioc, gpiof, gpiog;
    systick bound first, armv7-m.dtsi being included first).
- The order of entries in a linker section is not guaranteed; never assume
  an index means a particular device (gpioa came out as GPIO device 0).
  Use the lookup by name; the names are in board.h (LED_GPIO_NAME,
  CONSOLE_GPIO_NAME).

## Status (2026-11-23)
Steps 1-9 done, tested on the board and committed (step 9: bb0fa77,
539fa0c, a0649bc; rename to systime.* and -Werror: 5b66684).
Roadmap item 2 (QEMU lm3s6965evb) done: a8655df (skeleton), 361e7d5
(PL011), 72d6388 (PLL at 50 MHz, rate from RCC, Enter fix), then Part D
(audit, make check: 54e22f7). The STM32 build at 54e22f7 was flashed and
passed on the board (banner, blink, echo, Enter starts a new line).
- Part A: mach-lm3s and board/lm3s6965evb skeleton, run/debug targets.
  In gdb: vector table at 0 (SP 0x20010000), 1 device bound (systick,
  not probed), console_init() -ENODEV, silent panic("No console!").
- Part B: PL011 serial driver, row uart0 in board.c, driver chosen in
  board.mk. In gdb: RCGC1 0x1, IBRD 6, FBRD 50, LCR_H 0x70, CR 0x301; no
  guest_errors. No generic file changed for a second driver in an
  existing uclass.
- Part C: lm3s_clock_init() in the datasheet / driverlib order (bypass,
  XTAL + PWRDN/OEN clear, SYSDIV + USESYSDIV, wait for RIS PLLLRIS with a
  bounded loop, clear BYPASS). Checked: RCC 0x01CE0380, IBRD 27 / FBRD 8,
  SysTick LOAD 49999, mdelay(1000) = 1.000 s of host time. Enter fix in
  common/main.c (both boards; STM32 text 3740 -> 3744).
- Part D, the audit. Outside the two new directories step 2 changed: the
  new driver (pl011_serial.c, platdata/pl011_serial.h, as designed), the
  Makefile (run, debug, check: tooling), header guards renamed (shared
  names found by the second target), whitespace, and the Enter fix (a
  shared bug, not a porting cost). Zero lines of generic C changed for
  the port: drivers/core, the uclasses, console.c, systime.c, lib/,
  start.S, fault.c and link.ld took a new SoC as they were, and
  armv7m_systick.c runs on both SoCs unchanged (only its data row
  differs). The four-level split held.
Roadmap item 3 (clock uclass), in parts:
- Part A done (110ff6a): UCLASS_CLK, clk.h, clk-uclass.c; the lm3s_sysctl
  provider (item 2 Part C's PLL code moved into probe); pl011_serial is
  the first consumer; mach-lm3s/clock.c is a shim for SysTick and the
  banner; lm3s board_init() empty again. 3 devices bound, RCC
  0x01CE0380, RCGC1 0x1. LM3S text 3332 -> 3648.
- Part B done, tested on the board: the stm32f4_rcc provider at HSI 16
  MHz (no probe yet; enable refuses buses 3, 6 and > 7, the reserved
  words); stm32_gpio and stm32_serial converted; rcc row in board.c;
  mach-stm32f4/clock.c is a shim; mach/clock_ids.h deleted. Same output
  as before plus the rcc device: 5 devices bound. STM32 text 3744 ->
  3972. The first clock request comes from the LED port in board_init(),
  not the UART.
- Part C done: SysTick converted (clk_dev in its plat, set in both
  devices.c); both banners look up the console clock with
  CONSOLE_UART_CLK_DEV (board.h; the UART rows use it too); include/
  clock.h and both shims (mach-<soc>/clock.c) deleted: one clock API.
  drivers/core untouched through Parts A-C. In gdb the SysTick probe
  returns 0, LOAD 49999. Text: LM3S 3668, STM32 3992 (+20 each: the
  shims were smaller than the inline lookups replacing them).
- Part D done, tested on the board: the stm32f4_rcc probe runs 168 MHz
  from HSE. It computes and checks every factor before touching a
  register (M 4, N 168, P 2, Q 7: VCO 336 MHz, USB 48; 5 wait states;
  APB1 /4, APB2 /2; over 168 MHz is -EINVAL, still on HSI), then:
  HSE on, PWREN + VOS scale 1, FLASH_ACR read back (-EIO), prescalers
  while still on HSI, PLLCFGR read-modify-write (reserved bit 29), PLL
  on, switch; each wait bounded by a loop count (-ETIMEDOUT: no timer
  yet). get_rate per bus from the registers. On the board: CFGR
  0x0000940A, PLLCFGR 0x27402A04, FLASH_ACR 0x00000705; banner
  "Console clock: 84000000 Hz"; mdelay(1000) 1000 ms; 60 blinks in
  60 s; echo works. No consumer changed: the UART and SysTick took
  their new rates through get_rate. Text: STM32 3992 -> 4532, LM3S
  unchanged (3668). Item 3 done.
Roadmap item 4 (SPI, gyroscope as a child device), in parts:
- Part A done, tested on the board: UCLASS_SPI, include/spi.h, the
  stm32_spi bus driver; rows gpioc, gpiof, spi5 (8 devices bound; the
  linker put gpiof and gpioc before gpioa). A temporary test after the
  banner box calls the bus ops directly with a hand-made slave and
  prints WHO_AM_I = 0xd3 (ret 0); Part B replaces it with a child
  device. In gdb: SPI5 CR1 0x0000035F, SR 0x00000002, GPIOF MODER
  0x000A8000 / AFRL 0x50000000 / AFRH 0x00000055, GPIOC MODER
  0x00000004 / ODR 0x00000002, RCC APB2ENR 0x00100010, AHB1ENR
  0x00100065. Text: STM32 4532 -> 5200; LM3S text unchanged, bss +36
  (the uclass table has one slot per uclass id).
- Part B done, tested on the board: parent/child in the core (see
  Parent and child), spi_xfer(), UCLASS_GYRO, the l3gd20 driver (probe
  only), the gyro row (cs 0, mode 3, 10 MHz in its parent_plat); the
  Part A test replaced by "Gyro: ready" after the banner box. 9 devices
  bound, gyro Parent: spi5; CR1 still 0x0000035F, now from the row.
- Part C done, tested on the board: gyro.h, gyro-uclass.c, l3gd20 read
  (mdps), the main-loop line once a second; gpio_set_speed() and fast
  SPI pins (see the debugging lesson); CTRL_REG4 written first. 4/4
  resets "Gyro: ready"; at rest x ~100 y ~400 z ~450 mdps; all axes
  respond with both signs, up to 195 dps, no wrap. Text: STM32 5200 ->
  6144; LM3S 3668 -> 4188 (the core: device_find_by_name, pass 2,
  cycle walk, orphan; gyro_read and the loop are linked but never
  reached; spi_xfer is dropped by --gc-sections).
- Part D done: the audit, 6d998c7..c459d84, 24 files +1311 / -39.
  New as designed: the SPI and gyro uclasses and drivers (2 uclass
  ids). Board data: 4 rows (gpioc, gpiof, spi5, gyro), pins and their
  speed, 3 clock ids. The core changed on purpose (it was the subject):
  parent / parent_plat / orphan, device_set_parent(), dm_init() pass 2,
  parent-first probe, device_find_by_name(), the parent column. The GPIO
  uclass grew set_speed for a hardware reason, the first op added
  because a second user (SPI) needed what the first (the UART) did not.
  main.c got the gyro line. Untouched: start.S, fault.c, link.ld, lib/,
  console.c, systime.c, the clk / serial / timer uclasses, every older
  driver but stm32_gpio, all of mach-* and board/lm3s6965evb. Unlike
  item 2 (zero generic lines) the model was extended with a concept,
  not patched for a SoC; nothing that worked needed fixing. lm3s map:
  spi_xfer discarded; gyro_read linked (main.c calls it) but never
  reached, about 170 bytes.
  Decided: the gyro offset belongs in the gyro uclass (a calibrate
  operation, the offset in per-device uclass data), done in item 5a
  when uclasses get their own data; until then read returns the raw
  rate in mdps, uncorrected. The gyro line stays in main.c until
  aliases (item 5c) replace "gyro device 0". Item 4 done.
Roadmap item 5a (the uclass driver), in parts: A. the uclass driver
exists (name, id; bind needs one); B. per-device uclass data and the
pre_probe / post_probe hooks, first used for the gyro offset; C.
per-child data and child_pre_probe, first used by the SPI bus; D. the
audit. post_bind waits for a real user (5d / 5e).
- Part A done, tested on the board: struct uclass_driver, UCLASS_DRIVER,
  the .dm_uclasses section (0x30 on both boards: 6 rows of 8 bytes; lm3s
  links every uclass file too), uc_drv in struct uclass, -EPFNOSUPPORT
  at bind, names in the device list. Same boot otherwise (9 devices
  bound, Gyro: ready). Text: STM32 6144 -> 6292, LM3S 4188 -> 4352.
  The gdb check and the three experiments (remove the gyro, spi, lm3s
  serial row) were not run.
- Part B done, tested on the board: gyro UcPriv 0x200002a8 (after
  spi5's priv), every other device 0; at rest all values within +-333,
  mean of 11 lines x -57, y -21, z -73 mdps (was ~+100 / +400 / +450).
  Not checked: the boot pause, rotation, gdb steps, experiments (move
  at reset, no post_probe, read NULL). The device list's second line
  is now over 70 characters and wraps in minicom. Text: STM32 6292 ->
  6668, LM3S 4352 -> 4728 (the gyro hooks are linked on lm3s too).
  Parts A-C in one commit.
- Part C built (make check passes), NOT yet tested on the board:
  expected the same boot as Part B; in gdb gyro_pre_probe,
  spi_child_pre_probe, l3gd20_probe, gyro_post_probe hit in that order;
  dev->parent_priv 0x200002b8 = {cs 0, mode 3, max_hz 10000000}; with
  max_hz 0 or mode 4: -22 and l3gd20_probe never hit; with parent
  gpioc: -22 from spi_xfer, the hook never hit, parent_priv 0. grep:
  parent_plat is read only in spi_child_pre_probe. Text: STM32 6668 ->
  6820, LM3S 4728 -> 4876.
- Part D, the audit (94f31eb..working tree, one commit for A-C): 15
  source files, +507 / -49. The core gained a concept (uclass_driver,
  three data blocks, three hook points: uclass.h, device.h, uclass.c,
  device.c, link.ld); four uclasses got only their row (6-7 lines);
  first users gyro (gyro-uclass.c, gyro.h) and spi (spi-uclass.c,
  spi.h); stm32_spi.c changed 2 lines (the op's argument type).
  Untouched: start.S, fault.c, lib/, common/ (main.c too), both boards,
  mach-*, root.c, driver.c, every driver but stm32_spi, l3gd20.c
  included: the gyro got calibration and its slave settings got checked
  without a line of its driver changing. Text over 5a: STM32 6144 ->
  6820 (+676), LM3S 4188 -> 4876 (+688; it pays for hooks of uclasses
  it has no device in, see Known limitations). Thought questions
  answered: a gyro row with .id UCLASS_SPI makes the gyro unbindable
  (-96) and, if first in the section, gives spi5 the gyro's hooks
  (gyro_pre_probe passes on spi_ops.xfer, post_probe calls xfer with
  the wrong arguments): undefined behaviour from a clean build; a
  second calibration through gyro_read() would restore the raw offsets.
  Item 5a done: Part C passed on the board with item 5e's run (the
  tree-filled spi_slave_plat went through spi_child_pre_probe, Gyro:
  ready).
Roadmap item 5e (device tree instead of the C table; 5b folded in),
decided: a real FDT blob and our own reader (not OF_PLATDATA), the C
table kept working until the end, the STM32 first. Parts: A the blob
and the reader; B matching; C of_to_plat and phandles; D children from
the tree; E the rest, the rows removed, lm3s, audit. A was guided, B-E
implemented at once on request.
- Part A, tested on lm3s (QEMU) and against dtc: the three .dts levels,
  dt-bindings headers (clock IDs moved out of platdata), the cpp + dtc
  rule, common/dtb.S, lib/fdt.c. fdt_print() identical to dtc -O dts on
  both blobs (STM32 read by a host build of lib/fdt.c); -V 16 gives
  header -134; a 3-byte property keeps the padding; the .incbin
  dependency rebuilds the image. Text: STM32 6820 -> 10332, LM3S 4876
  -> 6484 (blob + reader).
- Parts B-E done, tested on both boards. lm3s in QEMU: 3 devices from
  the tree (uart0, systick, sysctl), plat on the heap, console clock
  50 MHz (crystal through the fixed-clock). STM32 bind and of_to_plat
  first checked on the host (real core, uclasses and drivers built for
  x86 with the real blob, no probes; needs -malign-data=abi, x86-64
  aligns 32-byte structs to 32 and gaps the linker arrays): 9 devices,
  every value equal to the old rows. On the board: same boot, Console
  clock 84000000 Hz, 9 bound, names from labels, plat 0x2000xxxx, gyro
  under spi5, Gyro: ready (WHO_AM_I 0xd3 = the I3G4250D the tree
  names), at rest within +-260 mdps, DT summary "13 nodes under /" (12
  + __symbols__). The boot prints the DT summary only, not the tree.
  Text: STM32 6820 -> 12852, LM3S 4876 -> 8812 (two blobs with
  __symbols__ and phandles, the reader, dm/read.h, of_to_plat in every
  driver). The rows and both mach-*/devices.c are gone; board.h keeps
  names and pins.
  Audit (since c148894, 5a's commit): the core gained the tree path
  (device_bind_node, device_bind_common shared with C rows, of_to_plat
  in probe, post_bind / child_post_bind / per_child_plat_auto,
  device_find_by_node, uclass_device_count, read.c); every driver
  gained of_match, of_to_plat, plat_auto (probes and ops unchanged but
  l3gd20's WHO_AM_I check); clk and gpio uclasses one helper each; spi
  uclass the child binding. Untouched: start.S, fault.c, link.ld,
  lib/ but fdt.c, console.c, systime.c, the gyro, serial and timer
  uclasses, every platdata struct. Item 5e done (d477536).
- After 5e, the C-table path removed (decided: no user left, and
  unexercised code cannot be trusted): struct device_info, DM_DEVICE,
  .dm_devices in link.ld, device_bind(), device_set_parent(),
  dev->orphan, dm_init()'s two passes, device_find_by_name(),
  driver_find(), DEV_NODE_NONE, the "(missing)" column; the shared bind
  helper folded into device_bind_node(); l3gd20 always has its chip.
  Same boot on lm3s (QEMU), same 9 devices and values on the host bind
  test, and on the board the same output as before, heap addresses
  included (udevice 48 -> 44 bytes, but calloc rounds to 8). Text: STM32
  12852 -> 12260, LM3S 8812 -> 8228.
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
  no way yet to choose among several (5c: /aliases, chosen).
- Bind errors printed before console_init() are lost (a device that did
  not bind is simply missing from the device list).
- A child node is bound only if its parent's uclass scans children in
  post_bind (only SPI does): a gyro node nested in gpioc would never be
  bound, silently. spi_xfer() still checks the parent's uclass.
- No list of children, no device removal, no root device.
- "Gyro device 0" is the gyro the main loop prints; the line is in
  generic main.c, so lm3s links gyro_read too (until item 5c).
- gpio_get_value() can return a negative error; board_led_toggle() does not
  test for it.
- The two uclass "get" functions duplicate their probe-and-return tail.
- Two UCLASS_DRIVER rows with the same id: the first in the section
  wins, silently (U-Boot does not check either).
- The stm32_spi driver has one chip select.
- The gyro reads a sample with no data-ready check (STATUS_REG). Its
  offset is measured once at probe: a board moved during the 0.4 s
  stores a wrong one (no check of the samples' spread); the offset
  also drifts with temperature after that.
- A uclass hook that is set is linked on every board, even one with no
  device in the uclass: the row is kept (KEEP) and points at it (lm3s
  links gyro_calibrate and spi_child_pre_probe). U-Boot compiles a
  uclass only when Kconfig enables it; here that would mean moving the
  gyro and spi uclass files from the generic SRCS to board.mk.
- Item 5a was not fully exercised: Part A's gdb check and experiments,
  Part B's rotation / boot pause / gdb / experiments, Part C's gdb
  checks and experiments (its board run passed with 5e).
- No device can be declared outside the tree (the C table, U-Boot's
  U_BOOT_DRVINFO, was removed: no user).
- Device names are DT labels (our choice, for board.h's lookups); a node
  without a label gets its node name ("gpio@40020000"). U-Boot uses the
  node name.
- The console's clock is written twice: in the .dts (usart1's clocks)
  and in board.h (CONSOLE_UART_CLK_DEV / _CLK, for the banner).
- fdt_node_by_phandle() and fdt_get_label() are linear searches of the
  whole tree, done at every bind / of_to_plat; dm_fdt() checks the
  header on every call. Fine for 13 nodes.
- dev_read_addr_index() takes only 1-cell addresses; no ranges (no
  simple-bus: every device node is a child of / or of a bus driver).
- No pinctrl in the tree: board.c still sets up the pins (names and
  pins in board.h).
- A changed Makefile flag does not rebuild the .dtb (or any object):
  make clean.
- A NULL dereference does not fault: address 0 aliases flash, a read
  returns the vector table, a write is dropped and sets FLASH_SR PGSERR /
  PGPERR (seen in step 9, priv_auto removed). A driver using dev->priv must
  declare .priv_auto; nothing checks it. An MPU no-access region at 0 would
  catch it.
- lm3s: get_rate knows only the PLL-through-SYSDIV state (0 otherwise).
- lm3s: UART0's pins (PA0/PA1) are not set up; fine in QEMU, needed on a
  real LM3S6965 (GPIO on lm3s skipped).

## Roadmap
The main purpose is learning the driver model; nothing else is in scope.
Agreed order:
1. Timer (SysTick): done (step 9).
2. Second target on QEMU (lm3s6965evb) to prove the four-level split:
   done (make check keeps both targets building).
3. Clock uclass, with the 168 MHz PLL from HSE as its first real use:
   done (Parts A-D).
4. SPI, with the L3GD20 gyroscope as a child device (parent/child):
   done (Parts A-D).
5. The rest of the driver model, in this order:
   a. The uclass driver (UCLASS_DRIVER): per-uclass hooks (post_bind,
      pre_probe, post_probe), per-device uclass data (per_device_auto)
      and per-child data (per_child_auto). The biggest gap: a uclass is
      today only an id and a file of functions. First use: the gyro
      offset (a calibrate operation, the offset in uclass data).
      Done (Parts A-D; Part C's board run passed with 5e); post_bind
      got its user in e; driver-level child hooks deferred.
   b. Matching by compatible string with driver_data: one driver, a
      table of chips (l3gd20 / i3g4250d). Done inside e.
   c. Sequence numbers and aliases (dev->seq, serial0, gyro0): replaces
      "device 0 is the console". Next; with the tree, an /aliases node
      (and /chosen stdout-path, as Linux).
   d. Children list, device_remove() and unbind (children before
      parents), and a dm tree print at boot.
   e. Optional: a device tree instead of the C table. Done (Parts A-E),
      tested on both boards.
Out of scope (decided 2026-11-22): command shell, booting an
application, SDRAM. Linked-list uclass storage when the fixed array
gets in the way.
