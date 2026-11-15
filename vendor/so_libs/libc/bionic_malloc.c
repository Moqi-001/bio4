#include <malloc.h>
#include <stdint.h>
#include <stdlib.h>

#include <so_common/so_logger.h>

#include "bionic_mman.h"

// 诊断：记录大块分配（>=4MB），用于判断堆是被单个巨块还是一片区域吃满。
#define BIG_ALLOC_THRESHOLD (1u * 1024 * 1024)

// 存活字节计数：malloc 加、free 减，用于区分「引擎真实需求」与「泄漏」
static unsigned int g_live_bytes  = 0;
static unsigned int g_alloc_calls = 0;
static unsigned int g_free_calls  = 0;

unsigned int malloc_live_bytes(void) {
    return g_live_bytes;
}

void *malloc_soloader(size_t size) {
    g_alloc_calls++;

    void *result = malloc(size);
    if (result) {
        g_live_bytes += malloc_usable_size(result);
    }
    if (!result && size >= 4096) {
        so_loge("malloc(%u) FAILED (live=%uKB, caller 0x%08X)", (unsigned)size, g_live_bytes / 1024,
                (unsigned)__builtin_return_address(0));
    } else if (size >= BIG_ALLOC_THRESHOLD) {
        so_loga("malloc(%u): %p live=%uKB (caller 0x%08X)", (unsigned)size, result,
                g_live_bytes / 1024, (unsigned)__builtin_return_address(0));
    }
    return result;
}

void *calloc_soloader(size_t num, size_t size) {
    g_alloc_calls++;
    void *result = calloc(num, size);
    if (result) {
        g_live_bytes += malloc_usable_size(result);
    }
    if (!result && num * size >= 4096) {
        so_loge("calloc(%u,%u) FAILED (caller 0x%08X)", (unsigned)num, (unsigned)size,
                (unsigned)__builtin_return_address(0));
    } else if (num * size >= BIG_ALLOC_THRESHOLD) {
        so_loga("calloc(%u,%u): %p live=%uKB (caller 0x%08X)", (unsigned)num, (unsigned)size, result,
                g_live_bytes / 1024, (unsigned)__builtin_return_address(0));
    }
    return result;
}

void *realloc_soloader(void *ptr, size_t size) {
    unsigned int old_usable = ptr ? malloc_usable_size(ptr) : 0;
    void        *result     = realloc(ptr, size);
    if (result) {
        unsigned int new_usable = malloc_usable_size(result);
        g_live_bytes = g_live_bytes - old_usable + new_usable;
    }
    if (!result && size >= 4096) {
        so_loge("realloc(0x%08X,%u) FAILED", (unsigned)ptr, (unsigned)size);
    } else if (size >= BIG_ALLOC_THRESHOLD) {
        so_loga("realloc(0x%08X,%u): %p live=%uKB (caller 0x%08X)", (unsigned)ptr, (unsigned)size, result,
                g_live_bytes / 1024, (unsigned)__builtin_return_address(0));
    }
    return result;
}
#include <malloc.h>

#include <so_common/so_logger.h>

#include "bionic_mman.h"

void *valloc_soloader(size_t size) {
    void *result = valloc(size);
    so_logd("valloc(%u): %p", size, result);
    return result;
}

unsigned int malloc_alloc_calls(void) { return g_alloc_calls; }
unsigned int malloc_free_calls(void)  { return g_free_calls; }

void free_soloader(void *ptr) {
    if (ptr) {
        unsigned int usable = malloc_usable_size(ptr);
        g_free_calls++;
        g_live_bytes = (g_live_bytes >= usable) ? g_live_bytes - usable : 0;
        if (usable >= BIG_ALLOC_THRESHOLD) {
            so_loga("free(0x%08X): usable=%uKB live=%uKB", (unsigned)ptr, usable / 1024, g_live_bytes / 1024);
        }
    }
    free(ptr);
}

/*
 * 引擎用 mallinfo() 判断"还能分配多少"（Black::SysRam::GetFreeSize 直接取
 * fordblks、GetUsedSize 取 uordblks）。newlib 的 arena 从启动起就等于整个堆，
 * 引擎因此以为有整堆可用，会一路缓存到堆满；bionic 的堆是按需增长，
 * 引擎在 Android 上看到的一直是较小的值。
 * 这里把报告值钳到一个预算内，让引擎按真实可承受的规模规划缓存。
 */
#define SYS_RAM_REPORT_BUDGET (128u * 1024 * 1024)

struct mallinfo mallinfo_soloader(void) {
    struct mallinfo mi = mallinfo();

    if ((unsigned)mi.arena > SYS_RAM_REPORT_BUDGET) {
        mi.arena = (int)SYS_RAM_REPORT_BUDGET;
    }
    int free_bytes = mi.arena - mi.uordblks;
    mi.fordblks    = free_bytes > 0 ? free_bytes : 0;

    so_logd("mallinfo(): arena=%d uordblks=%d fordblks=%d (reported)", mi.arena, mi.uordblks, mi.fordblks);
    return mi;
}
