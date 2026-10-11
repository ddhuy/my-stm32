# Lab 4: Bootloader + SDRAM application

A bootloader in internal flash initializes the clocks and the external SDRAM,
copies a packaged application image into SDRAM, and jumps to it.

## Build and flash

```sh
make                 # both programs
make bootloader      # build/bootloader/bootloader.bin
make application     # build/application/application.img
make inspect         # sizes and section layout
```

| File | Flash address |
|---|---|
| `build/bootloader/bootloader.bin` | `0x08000000` |
| `build/application/application.img` | `0x08020000` (`APP_FLASH_BASE`) |

Console: ST-LINK virtual COM port, 115200 8N1.

## Boot flow

1. `board_clock_init()`: 168 MHz from HSI via the PLL
2. `board_console_init()`: USART1 on PA9/PA10
3. `board_sdram_init()`: FMC + IS42S16400J on bank 2 (needs the clock first)
4. `mpu_map_app_region()`: SDRAM is execute-never by default, so map it as Normal memory
5. `sdram_memtest()`: data bus, address bus, byte lanes, every word (destructive; `BOOT_SDRAM_MEMTEST` in `main.c`)
6. `image_load()`: check the header at `APP_FLASH_BASE`, copy the payload to `APP_SDRAM_BASE`
7. `app_handoff()`: MSP = vector[0], jump to vector[1]; the application sets its own `VTOR`

## Source layout

Each layer may include only the layers below it.

```
common/       system design: memory_map.h, image_format.h     (the bootloader <-> app contract)
  ^
board/<BOARD>/   the PCB: pins, SDRAM chip, chosen clocks     #include "board.h"
  ^
drivers/      peripheral drivers, configured by the board    #include "clk/...", "ram/...", "serial/..."
  ^
soc/<SOC>/    the STM32F429: register maps, IRQ numbers      #include "soc/..."
  ^
arch/<ARCH>/  the Cortex-M4 core: SCB, NVIC, MPU, barriers   #include "arch/..."
```

Rules:

- `drivers/`, `soc/` and `arch/` never include `board.h` or `common/` headers.
  A driver gets everything board-specific through its config struct or arguments.
- Drivers do not enable their own clocks or route pins; the board does.
- `bootloader/` and `application/` are separate programs and cannot include each
  other's headers (each has its own include path in the Makefile).
- `ARCH`, `SOC` and `BOARD` in the Makefile choose the directories, so the
  include names (`"arch/..."`, `"soc/..."`, `"board.h"`) stay the same for any
  board, as with U-Boot's `asm/arch/`.

## One source of truth for addresses

`common/include/memory_map.h` defines the flash, SDRAM and SRAM layout. Everything
else derives from it:

- `bootloader/linker.ld.S` and `application/linker.ld.S` are run through the C
  preprocessor, so their `MEMORY` regions use the same macros as the C code.
  Constants use `_AC(x, u)` (see `arch/const.h`) because the linker does not
  accept the C `u` suffix.
- The bootloader's flash region ends at `APP_FLASH_BASE`, so a bootloader that
  grows past 128 KB fails to link.
- `scripts/package_image.py` gets the magic number and load address from the
  Makefile, which reads them from the headers.
