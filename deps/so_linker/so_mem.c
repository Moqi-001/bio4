#include <string.h>

#include <kubridge.h>
#include <psp2/kernel/sysmem.h>
#include <so_common/so_logger.h>

#include "so_internal.h"
#include "so_linker.h"

#ifndef SCE_KERNEL_MEMBLOCK_TYPE_USER_RX
#define SCE_KERNEL_MEMBLOCK_TYPE_USER_RX (0x0C20D050)
#endif

/*
typedef struct so_mmap_header {
    SceUID blockid;
} so_mmap_header;

#define MMAP_HEADER_ALIGN_SIZE (ALIGN_UP(sizeof(so_mmap_header), PAGE_SIZE))

void *so_mmap(size_t length) {
    size_t block_size = ALIGN_UP(MMAP_HEADER_ALIGN_SIZE + length, PAGE_SIZE);
    SceUID blockid    = sceKernelAllocMemBlock("ku_block", SCE_KERNEL_MEMBLOCK_TYPE_USER_RW, block_size, NULL);
    if (blockid < 0) {
        so_loge("[SOLINKER] so_mmap(%zu): failed to alloc memory! (error=0x%x)", length, blockid);
        return NULL;
    }

    so_mmap_header *header;
    if (sceKernelGetMemBlockBase(blockid, (void **)&header) < 0) {
        so_loge("[SOLINKER] so_mmap(%zu): failed to get base!", length);
        sceKernelFreeMemBlock(blockid);
        return NULL;
    }
    header->blockid = blockid;

    // 设置 RWX 权限
    if (kuKernelMemProtect((void *)header, block_size, KU_KERNEL_PROT_READ | KU_KERNEL_PROT_WRITE | KU_KERNEL_PROT_EXEC) < 0) {
        so_loge("[SOLINKER] so_mmap(%zu): failed to set protection!", length);
        sceKernelFreeMemBlock(blockid);
        return NULL;
    }

    void *addr = (void *)((uintptr_t)header + MMAP_HEADER_ALIGN_SIZE);

    so_logd("[SOLINKER] so_mmap(%zu): %p (blockid=0x%x)", length, addr, blockid);
    return addr;
}

void so_munmap(void *addr) {
    if (addr) {
        so_mmap_header *header = (so_mmap_header *)((uintptr_t)addr - MMAP_HEADER_ALIGN_SIZE);
        so_logd("[SOLINKER] so_munmap(%p): (blockid=0x%x)", addr, header->blockid);
        sceKernelFreeMemBlock(header->blockid);
    }
}
*/

void *so_mmap(size_t length) {
    size_t block_size = ALIGN_UP(length, PAGE_SIZE);
    SceUID blockid    = sceKernelAllocMemBlock("ku_block", SCE_KERNEL_MEMBLOCK_TYPE_USER_RW, block_size, NULL);
    if (blockid < 0) {
        so_loge("[SOLINKER] so_mmap(%zu): failed to alloc memory! (error=0x%x)", length, blockid);
        return NULL;
    }

    void *addr = NULL;
    if (sceKernelGetMemBlockBase(blockid, &addr) < 0) {
        so_loge("[SOLINKER] so_mmap(%zu): failed to get base!", length);
        sceKernelFreeMemBlock(blockid);
        return NULL;
    }

    // 设置 RWX 权限
    if (kuKernelMemProtect(addr, block_size, KU_KERNEL_PROT_READ | KU_KERNEL_PROT_WRITE | KU_KERNEL_PROT_EXEC) < 0) {
        so_loge("[SOLINKER] so_mmap(%zu): failed to set protection!", length);
        sceKernelFreeMemBlock(blockid);
        return NULL;
    }

    so_logd("[SOLINKER] so_mmap(%zu): %p (blockid=0x%x)", length, addr, blockid);
    return addr;
}

void so_munmap(void *addr) {
    if (addr) {
        SceUID blockid = sceKernelFindMemBlockByAddr(addr, 0);
        so_logd("[SOLINKER] so_munmap(%p): (blockid=0x%x)", addr, blockid);
        if (blockid >= 0) {
            sceKernelFreeMemBlock(blockid);
        }
    }
}

int so_msync(void *addr, size_t length) {
    if (!addr || length == 0) return -1;

    kuKernelFlushCaches(addr, length);
    so_logd("[SOLINKER] so_msync(%p,%zu): (blockid=0x%x)", addr, length);
    return 0;
}

void *so_memcpy(void *dst, const void *src, size_t n) {
    kuKernelCpuUnrestrictedMemcpy(dst, src, n);
    return dst;
}

void *so_memset(void *dst, int c, size_t n) {
    unsigned char chunk[4096];
    memset(chunk, c, n < sizeof(chunk) ? n : sizeof(chunk));

    unsigned char *p = (unsigned char *)dst;

    while (n >= sizeof(chunk)) {
        kuKernelCpuUnrestrictedMemcpy(p, chunk, sizeof(chunk));
        p += sizeof(chunk);
        n -= sizeof(chunk);
    }

    if (n > 0) {
        kuKernelCpuUnrestrictedMemcpy(p, chunk, n);
    }

    return dst;
}
