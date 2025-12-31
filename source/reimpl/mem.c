/*
 * Copyright (C) 2021      Andy Nguyen
 * Copyright (C) 2022      Rinnegatamante
 * Copyright (C) 2022-2023 Volodymyr Atamanenko
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#include "reimpl/mem.h"
#include "utils/logger.h"

#include "so_util/so_util.h"

#include <malloc.h>
#include <psp2/kernel/clib.h>
#include <string.h>

void *sceClibMemclr(void *dst, size_t len) {
    return sceClibMemset(dst, 0, len);
}

// void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offs) {
//     l_warn("mmap(%p, %i, %i, %i, %i, %li)", addr, length, prot, flags, fd, offs);

//     if (length <= 0) {
//         return MAP_FAILED;
//     }
//     // void* ret= malloc(length);
//     // l_debug("mmap return %p",ret);
//     // memset(ret, 0, length);
//     void* ret=allocMemBlock(length);
//     l_debug("mmap return %p",ret);
//     return ret;
// }

// int munmap(void *addr, size_t length) {
//     if (addr) free(addr);
//     return 0;
// }

#include <kubridge.h>

#ifndef SCE_KERNEL_MEMBLOCK_TYPE_USER_RX
#define SCE_KERNEL_MEMBLOCK_TYPE_USER_RX (0x0C20D050)
#endif
#define MAP_FIXED 0x10
#define PROT_EXEC 4

#define ALIGN(x, align) (((x) + ((align) - 1)) & ~((align) - 1))

void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset) {
    l_warn("mmap(%p, %i, %i, %i, %i, %li)", addr, length, prot, flags, fd, offset);

    int                             blockid;
    void                           *retval;
    SceKernelAllocMemBlockKernelOpt opt;
    memset(&opt, 0, sizeof(SceKernelAllocMemBlockKernelOpt));
    opt.size = sizeof(SceKernelAllocMemBlockKernelOpt);

    if (flags & MAP_FIXED) {
        opt.attr    = 0x1;
        opt.field_C = (SceUInt32)addr;
        blockid     = kuKernelAllocMemBlock("mmap", SCE_KERNEL_MEMBLOCK_TYPE_USER_RW, ALIGN(length, 0x1000), &opt);
    } else {
        if (prot & PROT_EXEC) {
            opt.attr     = 0x10000;
            opt.field_30 = 2;
            blockid      = kuKernelAllocMemBlock("mmap", SCE_KERNEL_MEMBLOCK_TYPE_USER_RX, ALIGN(length, 0x100000), &opt);
            sceKernelOpenVMDomain();
        } else {
            blockid = sceKernelAllocMemBlock("mmap", SCE_KERNEL_MEMBLOCK_TYPE_USER_RW, ALIGN(length, 0x1000), NULL);
        }
    }

    if (blockid < 0) {
        l_debug("Failed to allocate memory for %p %x %x %x\n", addr, length, prot, flags);
        return MAP_FAILED;
    }

    sceKernelGetMemBlockBase(blockid, &retval);
    l_debug("mmap return %p", retval);
    return retval;
}

int munmap(void *addr, size_t length) {
    int blockid = sceKernelFindMemBlockByAddr(addr, length);
    if (blockid < 0) {
        l_debug("Could not find memory %p with size %x to unmap.\n", addr, length);
        return -1;
    }
    sceKernelFreeMemBlock(blockid);
    return 0;
}
