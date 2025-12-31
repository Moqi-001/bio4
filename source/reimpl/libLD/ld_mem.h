#ifndef SOLOADED_LD_MEM_H
#define SOLOADED_LD_MEM_H

#include <stdint.h>

#include <psp2common/types.h>

#ifndef SCE_KERNEL_MEMBLOCK_TYPE_USER_RX
#define SCE_KERNEL_MEMBLOCK_TYPE_USER_RX (0x0C20D050)
#endif

typedef struct LdLoadMemEntry {
    struct LdLoadMemEntry *next;
    int                    flags;
    SceUID                 blockid;
    void                  *mem_base;
    size_t                 mem_size;
    uintptr_t              addr;
    size_t                 size;
} LdLoadMemEntry;

typedef struct LdLoadMemContext {
    LdLoadMemEntry *head;
    LdLoadMemEntry *tail;
    size_t          count;
    size_t          size;
    SceUID          blockid;
    uintptr_t       load_addr;
} LdLoadMemContext;

LdLoadMemContext *ldLoadMemGetContextFromData(const void *data);
LdLoadMemContext *ldLoadMemCreateContext(uintptr_t addr);
void              ldLoadMemDestroyContext(LdLoadMemContext *context);
LdLoadMemEntry   *ldLoadMemCreateEntry(uintptr_t addr, int flags, size_t size, size_t align);
void             *ldLoadMemDestroyEntry(LdLoadMemEntry *entry);
int               ldLoadMemAddEntry(LdLoadMemContext *context, LdLoadMemEntry *entry);
void              ldLoadMemFlushCaches(LdLoadMemContext *context);

#endif