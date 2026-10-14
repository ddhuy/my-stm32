#ifndef APP_HANDOFF_H
#define APP_HANDOFF_H

#include <stdint.h>

typedef enum {
    APP_HANDOFF_OK = 0,
    APP_HANDOFF_ERR_VECTOR_BASE,
    APP_HANDOFF_ERR_STACK,
    APP_HANDOFF_ERR_RESET_VECTOR
} app_handoff_status_t;


app_handoff_status_t app_handoff_validate(uint32_t vector_table);


/*
 * Starts the application whose vector table is at vector_table:
 * MSP = word 0, jump to word 1 (its reset handler). Does not return.
 *
 * Contract with the application:
 *   - the image is already copied and the MPU allows executing from it
 *   - no interrupts are enabled (the bootloader never enables any)
 *   - the application sets VTOR itself (see application/src/startup.c)
 */
__attribute__((noreturn))
void app_handoff(uint32_t vector_table);

#endif
