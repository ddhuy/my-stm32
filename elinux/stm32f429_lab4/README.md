# Lab 4: Bootloader + SDRAM application

A bootloader in internal flash initializes the clocks and the external SDRAM,
copies a packaged application image into SDRAM, and jumps to it.

## Build and flash

```sh
make                 # both programs
make bootloader      # build/bootloader/bootloader.bin
make application     # build/application/application.img
make inspect         # sizes and section layout
make disasm          # application and bootloader .lst files with C source
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
5. `sdram_memtest()`: data bus, address bus, byte lanes, every word
6. `image_load()`: check the header at `APP_FLASH_BASE`, copy the payload to `APP_SDRAM_BASE`, verify its CRC32
7. `app_handoff()`: MSP = vector[0], jump to vector[1]; the application sets its own `VTOR`

## Application CRC32

The packager writes CRC-32/ISO-HDLC (`zlib.crc32`) into the fourth 32-bit,
little-endian header field. It covers exactly the raw application payload,
excluding the 16-byte header and any flash padding. The reflected polynomial
is `0xEDB88320`, with initial value and final XOR both `0xFFFFFFFF`.
The check value for `123456789` is `0xCBF43926`.

After validating the header bounds and copying the payload, the bootloader
calculates CRC32 over the SDRAM copy. A mismatch makes `image_load()` return
false, so the bootloader halts with `boot_status = 0xE3` and prints
`bootloader: no valid application image`. CRC zero is not a validation bypass.
Rebuild and flash both programs; older images with the reserved zero CRC will
normally be rejected. CRC detects accidental corruption, not authenticity.

Run the host tests on Linux with a native C compiler:

```sh
python3 -m unittest discover -s tests -v
```

These check a known CRC vector, packaging and loading different payload lengths,
corrupted payloads and CRC fields, excluded padding, and invalid lengths.

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

## Useful commands:
- Start OpenOCD
```
openocd -f board/stm32f429disc1.cfg
```

- Start GDB
```
arm-none-eabi-gdb build/bootloader/bootloader.elf
```

- Connect to OpenOCD
```
target extended-remote :3333
```

- Flash the bootloader
```
load build/bootloader/bootloader.elf
```

- Flash the application
```
monitor flash write_image erase build/application/application.img 0x08020000 bin
```

- Reset the MCU
```
monitor reset halt
```

- Load application debug symbols
```
add-symbol-file build/application/application.elf
```

- Set breakpoint
```
break main
```

- Disable breakpoint #1
```
disable 1
```

- Delete all breakpoints
```
delete
```

- Some printing commands
```
info registers
p/x $pc
p tim2_irq_count
x/8wx 0xD0000000
```

- Disassemble with C source code
```sh
make disasm
```

Generates `build/application/application.lst` and `build/bootloader/bootloader.lst`
from their ELF files using `objdump -d -S`. Listings are regenerated when their
ELF changes, ignored by Git, and removed by `make clean`.
For later debugging, preserve the exact ELF and map files matching the flashed
binaries along with their source revision; listings are readable snapshots,
while GDB uses the ELF's symbols and debug information.
