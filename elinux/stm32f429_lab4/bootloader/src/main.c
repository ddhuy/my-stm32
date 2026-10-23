#include <stdbool.h>
#include <stdint.h>

#include "arch/cortex-m4/mpu.h"
#include "board/stm32f429i-disc1/board.h"
#include "board/stm32f429i-disc1/console.h"

#include "common/include/memory_map.h"

#include "drivers/flash/stm32_flash.h"

#include "app_handoff.h"
#include "boot_autoboot.h"
#include "boot_console.h"
#include "boot_metadata.h"
#include "boot_time.h"
#include "memtest.h"
#include "image_loader.h"


#define CONSOLE_BAUD    115200u

/* MPU region used for the application's SDRAM; it stays set after the handoff */
#define MPU_REGION_APP  0u


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

static bool boot_application(void)
{
    boot_slot_t selected_slot;

    if (!boot_select_slot(&selected_slot))
    {
        console_puts("No valid firmware in either slot.");
        return false;
    }

    /*
    * Validate the image and copy it from
    * Flash into SDRAM.
    *
    * Adapt this check to your current
    * image_status_t API.
    */
    image_status_t image_status = image_load_slot(selected_slot);
    if (image_status != IMAGE_OK)
    {
        console_puts("Failed to load selected firmware image.\n");
        console_puts("Image Loader: ");
        console_puthex(image_status);
        console_puts("\n");

        return false;
    }

    /* Configure SDRAM as executable memory. */
    mpu_map_app_region();

    /* Validate application vectors before handoff. */
    app_handoff_status_t handoff_status = app_handoff_validate(APP_SDRAM_BASE);
    if (handoff_status != APP_HANDOFF_OK)
    {
        console_puts("Failed to handoff to application image. \n");
        console_puthex(handoff_status);
        console_puts("\n");

        return false;
    }

    console_puts("Booting slot ");
    console_puts(selected_slot == BOOT_SLOT_A ? "A\n": "B\n");
    console_puts("Starting application...\n");
    console_flush();

    app_handoff(APP_SDRAM_BASE);

    __builtin_unreachable();
}

int main(void)
{
    if (!board_clock_init())
    {
        while (1)   /* no console without clocks */
        {
        }
    }

    board_console_init(CLOCK_PCLK2_HZ, CONSOLE_BAUD);
    console_puts("\nbootloader: clocks ok\n");

    if (!boot_time_init(CLOCK_HCLK_HZ))
        halt("bootloader: SysTick initialization failed\n");

    if (!board_sdram_init())
        halt("bootloader: SDRAM init failed\n");

    /* Test SDRAM on every bootup to make sure it works fine. */
    if (!sdram_memtest())
    {
        halt("bootloader: halted\n");
    }

    /*
     * Check Flash before starting the autoboot countdown.
     *
     * An invalid or incomplete firmware image should
     * enter recovery mode immediately.
     */
    boot_slot_t selected_slot;

    bool fw_available = boot_select_slot(&selected_slot);

    /* Autoboot */
    if (fw_available)
    {
        console_puts("Selected boot slot: ");
        console_puts(selected_slot == BOOT_SLOT_A ? "A\n" : "B\n");

        if (boot_autoboot_wait())
        {
            if (!boot_application())
            {
                console_puts("Autoboot failed. Entering console.\n");
            }
        }
    }
    else
    {
        console_puts("\nNo valid application in Flash.\n");
        console_puts(
            "Entering recovery console.\n"
            "Use 'update' to install firmware.\n");
    }

    /* Enter the command interface. */
    for (;;)
    {
        boot_console_action_t action = boot_console_run();

        if (action == BOOT_CONSOLE_BOOT)
        {
            if (!boot_application())
            {
                console_puts("Boot failed. Returning to recovery console.\n");
            }
        }
    }
}
