#include <string.h>

#include <sys/types.h>

#include <kubridge.h>
#include <so_common/so_logger.h>
#include <so_linker/so_linker.h>

#include "bionic_mman.h"

#define PAGE_SIZE 4096

#define ALIGN(x, align) (((x) + ((align) - 1)) & ~((align) - 1))

void *mmap_soloader(void *addr, size_t length, int prot, int flags, int fd, off_t offset) {
    void  *mem_data = NULL;
    size_t mem_size = ALIGN(length, PAGE_SIZE);

    SceUID blockid;
    if (flags & MAP_FIXED) {
        SceKernelAllocMemBlockKernelOpt opt;
        memset(&opt, 0, sizeof(SceKernelAllocMemBlockKernelOpt));
        opt.size    = sizeof(SceKernelAllocMemBlockKernelOpt);
        opt.attr    = 0x1;
        opt.field_C = (SceUInt32)addr;
        blockid     = kuKernelAllocMemBlock("mmap_block", SCE_KERNEL_MEMBLOCK_TYPE_USER_RW, mem_size, &opt);
    } else {
        blockid = sceKernelAllocMemBlock("mmap_block", SCE_KERNEL_MEMBLOCK_TYPE_USER_RW, mem_size, NULL);
    }

    if (blockid < 0) {
        so_loge("mmap(%p,%u,0x%x,0x%x,%d,%ld) failed: 0x%x", addr, length, prot, flags, fd, offset, blockid);
        return MAP_FAILED;
    }
    sceKernelGetMemBlockBase(blockid, &mem_data);

    SceUInt32 ku_prot = KU_KERNEL_PROT_NONE;
    if ((prot & PROT_READ) == PROT_READ)
        ku_prot |= KU_KERNEL_PROT_READ;
    if (prot & PROT_WRITE)
        ku_prot |= KU_KERNEL_PROT_WRITE;
    if (prot & PROT_EXEC)
        ku_prot |= KU_KERNEL_PROT_EXEC;
    kuKernelMemProtect(mem_data, mem_size, ku_prot);

    so_logd("mmap(%p,%u,0x%x,0x%x,%d,%ld): %p", addr, length, prot, flags, fd, offset, mem_data);
    return mem_data;
}

int munmap_soloader(void *addr, size_t length) {
    int blockid = sceKernelFindMemBlockByAddr(addr, 0);
    if (blockid < 0) {
        so_loge("munmap(%p,%u) error: Could not find memory!", addr, length);
        return -1;
    }

    sceKernelFreeMemBlock(blockid);
    so_logd("munmap(%p,%u): 0", addr, length);
    return 0;
}

int mprotect_soloader(void *addr, size_t length, int prot) {
    SceUInt32 ku_prot = KU_KERNEL_PROT_NONE;
    if ((prot & PROT_READ) == PROT_READ)
        ku_prot |= KU_KERNEL_PROT_READ;
    if ((prot & PROT_WRITE) == PROT_WRITE)
        ku_prot |= KU_KERNEL_PROT_WRITE;
    if ((prot & PROT_EXEC) == PROT_EXEC)
        ku_prot |= KU_KERNEL_PROT_EXEC;

    int result = kuKernelMemProtect(addr, length, ku_prot);
    so_logd("mprotect(%p,%u,0x%x): %d", addr, length, prot, result);
    return result;
}

int msync_soloader(void *addr, size_t length, int flags) {
    if (!addr || length == 0) return -1;

    so_logd("msync(%p,%u): 0", addr, length);
    // kuKernelFlushCaches(addr, length);
    return 0;
}
