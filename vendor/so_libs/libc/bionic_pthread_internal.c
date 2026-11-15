#include <stdlib.h>

#include <psp2/kernel/threadmgr.h>
#include <so_common/so_logger.h>

#include "bionic_pthread.h"

#define INIT_CACHE_CAPACITY 256

typedef struct cache_entry {
    void *key;
    void *value;
} cache_entry;

static int                   cache_capacity  = 0;
static cache_entry          *cache_slots     = NULL;
static int                  *cache_next_free = NULL;
static int                   cache_free_head = 0;
static SceKernelLwMutexWork  pthr_mutex;
static pthread_once_t_bionic pthr_once = BIONIC_PTHREAD_ONCE_INIT;

static void pthr_init() {
    int ret = sceKernelCreateLwMutex(&pthr_mutex, "pthr_mutex", 0, 0, NULL);
    if (ret < 0) {
        so_logf("pthr_init: Failed to create pthr mutex, error=0x%x", ret);
        return;
    }

    int capacity = INIT_CACHE_CAPACITY;

    cache_slots = malloc(capacity * sizeof(cache_entry));
    if (!cache_slots) {
        so_logf("pthr_init: Failed to alloc cache_slots!");
        return;
    }

    cache_next_free = malloc(capacity * sizeof(int));
    if (!cache_next_free) {
        so_logf("pthr_init: Failed to alloc cache_next_free!");
        return;
    }

    for (int i = 0; i < capacity; i++) {
        cache_slots[i].key   = NULL;
        cache_slots[i].value = NULL;
        cache_next_free[i]   = i + 1;
    }

    cache_capacity  = capacity;
    cache_free_head = 0;
}

#define PTHR_INIT   pthread_once_soloader(&pthr_once, pthr_init);
#define PTHR_LOCK   sceKernelLockLwMutex(&pthr_mutex, 1, NULL);
#define PTHR_UNLOCK sceKernelUnlockLwMutex(&pthr_mutex, 1);

static int ensure_cache_capacity() {
    if (cache_free_head < cache_capacity) {
        return 0;
    }

    int old_capacity = cache_capacity;
    int new_capacity = old_capacity * 2;

    cache_entry *slots = realloc(cache_slots, new_capacity * sizeof(cache_entry));
    if (!slots) {
        so_logf("ensure_cache_capacity: Failed to realloc cache slots!");
        return -1;
    }
    cache_slots = slots;

    int *next_free = realloc(cache_next_free, new_capacity * sizeof(int));
    if (!next_free) {
        so_logf("ensure_cache_capacity: Failed to alloc cache next_free!");
        return -1;
    }
    cache_next_free = next_free;

    for (int i = old_capacity; i < new_capacity; i++) {
        cache_slots[i].key   = NULL;
        cache_slots[i].value = NULL;
        cache_next_free[i]   = i + 1;
    }

    cache_capacity = new_capacity;

    // so_logd("ensure_cache_capacity: realloc %d=>%d", old_capacity, new_capacity);
    return 0;
}

int pthr_add_cache(void *key, void *value) {
    if (!key || !value) return -1;

    PTHR_INIT
    PTHR_LOCK

    ensure_cache_capacity();

    int index                = cache_free_head;
    cache_slots[index].key   = key;
    cache_slots[index].value = value;
    cache_free_head          = cache_next_free[cache_free_head];

    PTHR_UNLOCK

    // so_logd("pthr_add_cache(%p,%p): %d", key, value, index);
    return index;
}

void *pthr_remove_cache(void *key, int id) {
    PTHR_INIT
    PTHR_LOCK

    int index = id;
    if (!key || index < 0 || index >= cache_capacity) {
        PTHR_UNLOCK
        return NULL;
    }

    void *value = NULL;
    if (cache_slots[index].key == key) {
        value = cache_slots[index].value;

        cache_slots[index].key   = NULL;
        cache_slots[index].value = NULL;
        cache_next_free[index]   = cache_next_free[cache_free_head];
        cache_free_head          = index;
    }

    PTHR_UNLOCK

    // so_logd("pthr_remove_cache(%p,%d): %p", key, id, value);
    return value;
}

void *pthr_find_cache(void *key, int id) {
    PTHR_INIT
    PTHR_LOCK

    int index = id;
    if (!key || index < 0 || index >= cache_capacity) {
        PTHR_UNLOCK
        return NULL;
    }

    void *value = NULL;
    if (cache_slots[index].key == key) {
        value = cache_slots[index].value;
    }

    PTHR_UNLOCK

    // so_logd("pthr_find_cache(%p,%d): %p", key, id, value);
    return value;
}
