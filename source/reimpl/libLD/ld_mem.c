#include <stdlib.h>
#include <string.h>

#include <kubridge.h>
#include <so_util/so_util.h>

#include "ld_mem.h"
#include "utils/logger.h"

#define ALIGN(x, align) (((x) + ((align) - 1)) & ~((align) - 1))

#define LdLoadMemContextAlignSize (ALIGN(sizeof(LdLoadMemContext), 0x1000))

LdLoadMemContext *ldLoadMemGetContextFromData(const void *data) {
    return data ? (LdLoadMemContext *)((uintptr_t)data - LdLoadMemContextAlignSize) : NULL;
}

LdLoadMemContext *ldLoadMemCreateContext(uintptr_t addr) {
    l_debug("ldLoadMemCreateContext(0x%x)", addr);

    LdLoadMemContext *context  = NULL;
    size_t            mem_size = LdLoadMemContextAlignSize;

    SceKernelAllocMemBlockKernelOpt opt;
    memset(&opt, 0, sizeof(SceKernelAllocMemBlockKernelOpt));
    opt.size       = sizeof(SceKernelAllocMemBlockKernelOpt);
    opt.attr       = 0x1;
    opt.field_C    = addr - mem_size;
    SceUID blockid = kuKernelAllocMemBlock("rw_block", SCE_KERNEL_MEMBLOCK_TYPE_USER_RW, mem_size, &opt);
    if (blockid < 0) {
        l_debug("ldLoadMemCreateContext(0x%x) failed: err_code=0x%x", addr, blockid);
        return NULL;
    }

    sceKernelGetMemBlockBase(blockid, (void **)&context);
    memset(context, 0, sizeof(LdLoadMemContext));
    context->blockid   = blockid;
    context->load_addr = (uintptr_t)context + mem_size;

    l_debug("ldLoadMemCreateContext success: context=%p, load_addr=0x%x", context, context->load_addr);
    return context;
}

void ldLoadMemDestroyContext(LdLoadMemContext *context) {
    if (context) {
        l_debug("ldLoadMemDestroyContext(%p), load_addr=0x%x, count=%u, size=0x%x",
                context, context->load_addr, context->count, context->size);
        LdLoadMemEntry *entry = context->head;
        while (entry) {
            LdLoadMemEntry *next = entry->next;
            l_debug("ldLoadMemDestroyEntry(%p): mem_base=%p, mem_size=%u", entry, entry->mem_base, entry->mem_size);
            sceKernelFreeMemBlock(entry->blockid);
            free(entry);
            entry = next;
        }
        sceKernelFreeMemBlock(context->blockid);
    }
}

LdLoadMemEntry *ldLoadMemCreateEntry(uintptr_t addr, int flags, size_t size, size_t align) {
    l_debug("ldLoadMemCreateEntry(0x%x,\"%s\",0x%x,0x%x)", addr,
            (flags == SCE_KERNEL_MEMBLOCK_TYPE_USER_RX) ? "rx" : "rw",
            size, align);

    LdLoadMemEntry *entry = calloc(1, sizeof(LdLoadMemEntry));
    if (!entry)
        return NULL;

    uintptr_t mem_addr = addr & ~(align - 1);
    size_t    mem_size = ALIGN(size + (addr - mem_addr), align);

    SceKernelAllocMemBlockKernelOpt opt;
    memset(&opt, 0, sizeof(SceKernelAllocMemBlockKernelOpt));
    opt.size       = sizeof(SceKernelAllocMemBlockKernelOpt);
    opt.attr       = 0x1;
    opt.field_C    = mem_addr;
    SceUID blockid = kuKernelAllocMemBlock((flags == SCE_KERNEL_MEMBLOCK_TYPE_USER_RX) ? "rx_block" : "rw_block",
                                           flags, mem_size, &opt);
    if (blockid < 0) {
        l_debug("ldLoadMemCreateEntry failed: err_code=0x%x, mem_addr=0x%x, mem_size=0x%x", blockid, mem_addr, mem_size);
        free(entry);
        return NULL;
    }

    sceKernelGetMemBlockBase(blockid, &entry->mem_base);
    entry->mem_size = mem_size;
    entry->blockid  = blockid;
    entry->flags    = flags;
    entry->addr     = addr;
    entry->size     = size;

    l_debug("ldLoadMemCreateEntry success: entry=%p, mem_base=%p, mem_size=0x%x, mem_end=%p",
            entry, entry->mem_base, entry->mem_size, (char *)entry->mem_base + mem_size);
    return entry;
}

void *ldLoadMemDestroyEntry(LdLoadMemEntry *entry) {
    if (entry) {
        l_debug("ldLoadMemDestroyEntry(%p): mem_base=%p, mem_size=0x%x", entry, entry->mem_base, entry->mem_size);
        sceKernelFreeMemBlock(entry->blockid);
        free(entry);
    }
}

int ldLoadMemAddEntry(LdLoadMemContext *context, LdLoadMemEntry *entry) {
    if (!context || !entry)
        return -1;

    if (!context->head) {
        context->head = entry;
        context->tail = entry;
    } else {
        context->tail->next = entry;
        context->tail       = entry;
    }

    context->count++;
    context->size += entry->mem_size;

    l_debug("ldLoadMemAddEntry(%p,%p): mem_base=%p, mem_size=0x%x", context, entry, entry->mem_base, entry->mem_size);
    return 0;
}

void ldLoadMemFlushCaches(LdLoadMemContext *context) {
    if (context) {
        l_debug("ldLoadMemFlushCaches(%p): load_addr=0x%x", context, context->load_addr);
        LdLoadMemEntry *entry = context->head;
        while (entry) {
            if (entry->flags == SCE_KERNEL_MEMBLOCK_TYPE_USER_RX) {
                l_debug("kuKernelFlushCaches(%p,0x%x)", (void *)entry->addr, entry->size);
                kuKernelFlushCaches((void *)entry->addr, entry->size);
            }
            entry = entry->next;
        }
    }
}
