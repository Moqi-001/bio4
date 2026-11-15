#ifndef BIONIC_MALLOC_H
#define BIONIC_MALLOC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <malloc.h>
#include <stddef.h>

void *valloc_soloader(size_t size);
void *malloc_soloader(size_t size);
void *calloc_soloader(size_t num, size_t size);
void *realloc_soloader(void *ptr, size_t size);
void  free_soloader(void *ptr);
unsigned int malloc_live_bytes(void);
unsigned int malloc_alloc_calls(void);
unsigned int malloc_free_calls(void);
struct mallinfo mallinfo_soloader(void);

#ifdef __cplusplus
};
#endif

#endif