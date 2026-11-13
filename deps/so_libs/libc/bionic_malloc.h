#ifndef BIONIC_MALLOC_H
#define BIONIC_MALLOC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

void *valloc_soloader(size_t size);

#ifdef __cplusplus
};
#endif

#endif