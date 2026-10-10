#ifndef IMAGE_LOADER_H
#define IMAGE_LOADER_H

#include <stdbool.h>
#include "image_format.h"

bool image_header_validate(const app_image_header_t *header);
bool image_load(void);

#endif