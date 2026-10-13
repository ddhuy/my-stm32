#ifndef APP_STARTUP_H
#define APP_STARTUP_H

#include <stdint.h>

extern const uintptr_t vector_table[];

void reset_handler(void);
void default_handler(void);


#endif
