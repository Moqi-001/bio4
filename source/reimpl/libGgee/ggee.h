#ifndef SOLOADED_GGEE_H
#define SOLOADED_GGEE_H

#include <stdint.h>

extern int (*ggeeLibExpand)(void *compressed_data, uint32_t compressed_size, void *expanded_data, uint32_t *expanded_size);

int ggeeLibExpandFile(const char *srcpath, const char *dstpath);

#endif