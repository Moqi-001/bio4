#include <string.h>

#include <sys/types.h>

#include <kubridge.h>
#include <so_common/so_logger.h>
#include <so_linker/so_linker.h>

#include "bionic_mman.h"

#define ALIGN(x, align) (((x) + ((align) - 1)) & ~((align) - 1))

/*
void *mmap_soloader(void *addr, size_t length, int prot, int flags, int fd, off_t offs) {
    so_logw("mmap_soloader(%p, %d, %d, %d, %d, %li)", addr, length, prot, flags, fd, offs);

    if (length <= 0) {
        return MAP_FAILED;
    }
    void *ret = malloc(length);
    if (!ret) {
        return MAP_FAILED;
    }
    memset(ret, 0, length);
    so_logd("mmap_soloader return %p", ret);
    return ret;
}

int munmap_soloader(void *addr, size_t length) {
    if (addr) free(addr);
    return 0;
}
*/

void *mmap_soloader(void *addr, size_t length, int prot, int flags, int fd, off_t offset) {
    void  *mem_data = NULL;
    size_t mem_size = ALIGN(length, 0x1000);

    SceKernelAllocMemBlockKernelOpt opt;
    memset(&opt, 0, sizeof(SceKernelAllocMemBlockKernelOpt));
    opt.size = sizeof(SceKernelAllocMemBlockKernelOpt);
    if (flags & MAP_FIXED) {
        opt.attr    = 0x1;
        opt.field_C = (SceUInt32)addr;
    } else {
        opt.attr     = 0x10000;
        opt.field_30 = 2;
    }

    SceUID blockid = kuKernelAllocMemBlock("mmap_soloader", SCE_KERNEL_MEMBLOCK_TYPE_USER_RW, mem_size, &opt);
    if (blockid < 0) {
        so_logd("mmap_soloader(%p,%zu,0x%x,0x%x,%d,%ld) failed: 0x%x", addr, length, prot, flags, fd, offset, blockid);
        return MAP_FAILED;
    }
    sceKernelGetMemBlockBase(blockid, &mem_data);

    SceUInt32 ku_prot = KU_KERNEL_PROT_NONE;
    if (prot & PROT_READ)
        ku_prot |= KU_KERNEL_PROT_READ;
    if (prot & PROT_WRITE)
        ku_prot |= KU_KERNEL_PROT_WRITE;
    if (prot & PROT_EXEC)
        ku_prot |= KU_KERNEL_PROT_EXEC;
    kuKernelMemProtect(mem_data, mem_size, ku_prot);

    so_logd("mmap_soloader(%p,%zu,0x%x,0x%x,%d,%ld): %p", addr, length, prot, flags, fd, offset, mem_data);
    return mem_data;
}

int munmap_soloader(void *addr, size_t length) {
    int blockid = sceKernelFindMemBlockByAddr(addr, 0);
    if (blockid < 0) {
        so_logd("munmap_soloader(%p,%zu) error: Could not find memory!", addr, length);
        return -1;
    }

    sceKernelFreeMemBlock(blockid);
    so_logd("munmap_soloader(%p,%zu): 0", addr, length);
    return 0;
}

int mprotect_soloader(void *addr, size_t length, int prot) {
    SceUInt32 ku_port = KU_KERNEL_PROT_NONE;
    if ((prot & PROT_READ) == PROT_READ)
        ku_port |= PROT_READ;
    if ((prot & PROT_WRITE) == PROT_WRITE)
        ku_port |= KU_KERNEL_PROT_WRITE;
    if ((prot & PROT_EXEC) == PROT_EXEC)
        ku_port |= KU_KERNEL_PROT_EXEC;

    int result = kuKernelMemProtect(addr, length, ku_port);
    so_logd("mprotect_soloader(%p,%zu,0x%x): %d", addr, length, prot, result);
    return result;
}

int msync_soloader(void *addr, size_t length, int flags) {
    if (!addr || length == 0) return -1;

    kuKernelFlushCaches(addr, length);
    so_logd("[SOLINKER] msync_soloader(%p,%u): 0", addr, length);
    return 0;
}
