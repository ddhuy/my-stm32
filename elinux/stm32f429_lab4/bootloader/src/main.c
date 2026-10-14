#include <stdbool.h>
#include <stdint.h>

#include "arch/cortex-m4/mpu.h"
#include "board/stm32f429i-disc1/board.h"

#include "drivers/serial/stm32_usart.h"

#include "common/include/memory_map.h"
#include "common/include/image_format.h"

#include "image_loader.h"
#include "memtest.h"
#include "app_handoff.h"


#define CONSOLE_BAUD    115200u

/* MPU region used for the application's SDRAM; it stays set after the handoff */
#define MPU_REGION_APP  0u


volatile uint32_t boot_status;


static void console_puts(const char *s)
{
    stm32_usart_puts(CONSOLE_USART, s);
}

static void console_puthex(uint32_t value)
{
    static const char digits[] = "0123456789ABCDEF";
    char buf[11] = "0x";

    for (unsigned i = 0; i < 8u; ++i)
        buf[2u + i] = digits[(value >> (28u - 4u * i)) & 0xFu];
    buf[10] = '\0';

    console_puts(buf);
}

static __attribute__((noreturn)) void halt(const char *why)
{
    console_puts(why);
    while (1)
    {
        __asm volatile("nop");
    }
}

/*
 * The Cortex-M4 default memory map marks 0xA0000000..0xDFFFFFFF as Device
 * memory with execute-never, so code in SDRAM would fault. Running the
 * application from SDRAM is this system's design, so the bootloader maps
 * the application region as Normal, executable memory.
 */
static void mpu_map_app_region(void)
{
    _Static_assert(APP_SDRAM_SIZE == 8u * 1024u * 1024u,
                   "MPU_RASR_SIZE_8MB below must match APP_SDRAM_SIZE");

    mpu_disable();
    /* XN=0, AP=011 full access, TEX=001 Normal non-cacheable, S/C/B=0 */
    mpu_set_region(MPU_REGION_APP, APP_SDRAM_BASE,
                   MPU_RASR_AP_FULL | MPU_RASR_TEX_NORMAL | MPU_RASR_SIZE_8MB |
                   MPU_RASR_ENABLE);
    mpu_enable(MPU_CTRL_PRIVDEFENA);
}

static bool sdram_memtest(void)
{
    memtest_result_t result;

    console_puts("bootloader: testing SDRAM ");
    console_puthex(APP_SDRAM_BASE);
    console_puts(", ");
    console_puthex(APP_SDRAM_SIZE);
    console_puts(" bytes\n");

    if (memtest_run(APP_SDRAM_BASE, APP_SDRAM_SIZE, &result))
    {
        console_puts("bootloader: SDRAM test passed\n");
        return true;
    }

    console_puts("bootloader: SDRAM test FAILED: ");
    console_puts(result.test);
    console_puts("\n    address  ");
    console_puthex(result.address);
    console_puts("\n    expected ");
    console_puthex(result.expected);
    console_puts("\n    actual   ");
    console_puthex(result.actual);
    console_puts("\n    diff     ");
    console_puthex(result.expected ^ result.actual);   /* the bits that are wrong */
    console_puts("\n");
    return false;
}

int main(void)
{
    boot_status = 1u;

    if (!board_clock_init())
    {
        boot_status = 0xE1u;
        while (1)   /* no console without clocks */
        {
        }
    }

    boot_status = 2u;

    board_console_init(CLOCK_PCLK2_HZ, CONSOLE_BAUD);
    console_puts("\nbootloader: clocks ok\n");

    if (!board_sdram_init())
        halt("bootloader: SDRAM init failed\n");

    boot_status = 3u;

    /* Destroys SDRAM contents, so it must run before image_load() */
    if (!sdram_memtest()) {
        boot_status = 0xE2u;
        halt("bootloader: halted\n");
    }

    boot_status = 4u;

    image_status_t image_status = image_load();
    if (image_status != IMAGE_OK) {
        boot_status = 0xE3u;

        console_puts("[ERR] bootloader: failed to load application image\n");
        console_puts("[ERR] bootloader: image_load() returned ");
        console_puthex(image_status);
        console_puts("\n");

        halt("[ERR] bootloader: halted\n");
    }

    boot_status = 5u;

    mpu_map_app_region();

    boot_status = 6u;

    console_puts("bootloader: starting application\n");
    stm32_usart_flush(CONSOLE_USART);

    app_handoff_status_t handoff_status = app_handoff_validate(APP_SDRAM_BASE);
    if (handoff_status != APP_HANDOFF_OK) {
        boot_status = 0xE4u;

        console_puts("[ERR] bootloader: failed to handoff to application image\n");
        console_puts("[ERR] bootloader: app_handoff_validate() returned ");
        console_puthex(handoff_status);
        console_puts("\n");

        halt("bootloader: halted\n");
    }

    app_handoff(APP_SDRAM_BASE);

    // The bootloader has nothing else to do, so we enter an infinite loop.
    while (1)
    {
        __asm volatile("nop");
    }
}
