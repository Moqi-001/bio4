#ifndef BIONIC_MMAN_H
#define BIONIC_MMAN_H

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/types.h>

#define MAP_FAILED    ((void *)-1)
#define MAP_SHARED    0x01
#define MAP_PRIVATE   0x02
#define MAP_FIXED     0x10
#define MAP_ANONYMOUS 0x20

#define PROT_NONE  0x0
#define PROT_READ  0x1
#define PROT_WRITE 0x2
#define PROT_EXEC  0x4

void *mmap_soloader(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
int   munmap_soloader(void *addr, size_t length);
int   mprotect_soloader(void *addr, size_t length, int prot);
int   msync_soloader(void *addr, size_t length, int flags);

#ifdef __cplusplus
};
#endif

#endif