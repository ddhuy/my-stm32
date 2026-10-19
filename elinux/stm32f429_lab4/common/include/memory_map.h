#ifndef MEMORY_MAP_H
#define MEMORY_MAP_H

/*
 * System memory layout: the contract between the bootloader and the
 * application. Everything else derives from these values:
 *   - bootloader/linker.ld.S and application/linker.ld.S (MEMORY regions)
 *   - the bootloader's image checks and MPU setup
 *   - scripts/package_image.py (the Makefile passes the load address)
 * Safe to include from linker scripts (see arch/const.h).
 *
 * Flash (2 MB):
 *   0x08000000  sectors 0..4  bootloader, at most 128 KB (enforced by its linker script)
 *   0x08020000  sectors 5..   packaged application image: header + payload
 */

#include "arch/cortex-m4/const.h"
#include "soc/stm32f429/periph_base.h"
#include "board/stm32f429i-disc1/board.h"

/* Flash */
#define BOOT_FLASH_BASE   FLASH_BASE
#define APP_FLASH_BASE    _AC(0x08020000, u)
#define APP_FLASH_END     (FLASH_BASE + FLASH_SIZE)
#define BOOT_FLASH_SIZE   (APP_FLASH_BASE - BOOT_FLASH_BASE)

/* The application runs from the board's external SDRAM */
#define APP_SDRAM_BASE    SDRAM_BASE
#define APP_SDRAM_SIZE    SDRAM_SIZE

/*
 * Internal SRAM (192 KB at 0x20000000). The bootloader uses all of it; the
 * application takes it over after the handoff and uses the first 128 KB
 * for .data, .bss and its stack.
 */
#define BOOT_SRAM_BASE    SRAM_BASE
#define BOOT_SRAM_SIZE    SRAM_SIZE
#define APP_SRAM_BASE     SRAM_BASE
#define APP_SRAM_SIZE     (_AC(128, u) * 1024)

/*
 * Initial firmware-update policy:
 *
 * Sectors 0-4: reserved for bootloader.
 * Sector 5:    application image storage.
 *
 * We initially permit updates to sector 5 only.
 */
#define APP_FLASH_FIRST_SECTOR   _AC(5, u)
#define APP_FLASH_SECTOR_SIZE    _AC(0x20000, u)  /* 128 KiB */

#define APP_UPDATE_FLASH_END \
    (APP_FLASH_BASE + APP_FLASH_SECTOR_SIZE)

#define APP_UPDATE_FLASH_SIZE \
    (APP_UPDATE_FLASH_END - APP_FLASH_BASE)


#endif
