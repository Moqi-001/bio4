#include <string.h>

#include <kubridge.h>
#include <psp2/kernel/sysmem.h>
#include <so_common/so_logger.h>

#include "so_internal.h"
#include "so_linker.h"

int so_cacheflush(void *addr, size_t length) {
    if (!addr || length == 0) {
        return -1;
    }

    so_logd("[SOLINKER] so_cacheflush(%p,%u)", addr, length);
    kuKernelFlushCaches(addr, length);
    return 0;
}

void *so_mmap(size_t length) {
    size_t block_size = ALIGN_UP(length, PAGE_SIZE);
    SceUID blockid    = sceKernelAllocMemBlock("mmap_block", SCE_KERNEL_MEMBLOCK_TYPE_USER_RW, block_size, NULL);
    if (blockid < 0) {
        so_loge("[SOLINKER] so_mmap(%u): failed to alloc memory! (error=0x%x)", length, blockid);
        return NULL;
    }

    void *addr = NULL;
    if (sceKernelGetMemBlockBase(blockid, &addr) < 0) {
        so_loge("[SOLINKER] so_mmap(%u): failed to get base!", length);
        sceKernelFreeMemBlock(blockid);
        return NULL;
    }

    // 设置 RWX 权限
    if (kuKernelMemProtect(addr, block_size, KU_KERNEL_PROT_READ | KU_KERNEL_PROT_WRITE | KU_KERNEL_PROT_EXEC) < 0) {
        so_loge("[SOLINKER] so_mmap(%u): failed to set protection!", length);
        sceKernelFreeMemBlock(blockid);
        return NULL;
    }

    so_logd("[SOLINKER] so_mmap(%u): %p (blockid=0x%x)", length, addr, blockid);
    return addr;
}

void so_munmap(void *addr) {
    if (addr) {
        SceUID blockid = sceKernelFindMemBlockByAddr(addr, 0);
        if (blockid >= 0) {
            sceKernelFreeMemBlock(blockid);
            so_logd("[SOLINKER] so_munmap(%p): (blockid=0x%x)", addr, blockid);
        } else {
            so_loge("[SOLINKER] so_munmap(%p): error! (0x%x)", addr, blockid);
        }
    }
}

int so_mprotect(void *addr, size_t length, int prot) {
    SceUInt32 ku_prot = KU_KERNEL_PROT_NONE;
    if ((prot & PROT_READ) == PROT_READ) {
        ku_prot |= KU_KERNEL_PROT_READ;
    }
    if ((prot & PROT_WRITE) == PROT_WRITE) {
        ku_prot |= KU_KERNEL_PROT_WRITE;
    }
    if ((prot & PROT_EXEC) == PROT_EXEC) {
        ku_prot |= KU_KERNEL_PROT_EXEC;
    }

    int result = kuKernelMemProtect(addr, length, ku_prot);
    so_logd("[SOLINKER] so_mprotect(%p,%u,0x%x): %d", addr, length, prot, result);
    return result;
}
