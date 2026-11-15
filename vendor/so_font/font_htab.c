#include <stdlib.h>
#include <string.h>

#include "font_htab.h"

typedef struct htab_entry {
    uint32_t key;
    void    *value;
} htab_entry;

typedef struct font_htab {
    size_t      size;
    size_t      used;
    htab_entry *entries;
} font_htab;

#define INT_HTAB_MAX_LOAD (70)  // over 100

static inline uint32_t FNV_1a(uint32_t key) {
    uint8_t *bytes = (uint8_t *)&key;
    uint32_t hash  = 2166136261U;
    hash           = (16777619U * hash) ^ bytes[0];
    hash           = (16777619U * hash) ^ bytes[1];
    hash           = (16777619U * hash) ^ bytes[2];
    hash           = (16777619U * hash) ^ bytes[3];
    return hash;
}

static size_t next_pow2(size_t n) {
    size_t ret = 1;
    while (ret < n) ret <<= 1;
    return ret;
}

font_htab *font_htab_new(size_t size) {
    font_htab *htab = malloc(sizeof(font_htab));
    if (!htab)
        return NULL;

    htab->size = next_pow2(size);
    htab->used = 0;

    htab->entries = calloc(htab->size, sizeof(*htab->entries));
    if (!htab->entries) {
        free(htab);
        return NULL;
    }

    return htab;
}

void font_htab_free(font_htab *htab) {
    for (size_t i = 0; i < htab->size; i++) {
        if (htab->entries[i].value != NULL)
            free(htab->entries[i].value);
    }
    free(htab);
}

static inline void _htab_insert(font_htab *htab, uint32_t key, void *value) {
    unsigned int mask = htab->size - 1;
    unsigned int idx  = FNV_1a(key) & mask;

    /* Open addressing, linear probing */
    while (htab->entries[idx].value != NULL) {
        idx = (idx + 1) & mask;
    }

    htab->entries[idx].key   = key;
    htab->entries[idx].value = value;
    htab->used++;
}

int font_htab_resize(font_htab *htab, size_t new_size) {
    htab_entry *old_entries = htab->entries;
    size_t      old_size    = htab->size;

    htab_entry *new_entries = calloc(new_size, sizeof(htab_entry));
    if (!new_entries) {
        return -1;
    }

    htab->entries = new_entries;
    htab->size    = new_size;
    htab->used    = 0;
    for (size_t i = 0; i < old_size; i++) {  // 旧表数据插入进新表
        if (old_entries[i].value != NULL) {
            _htab_insert(htab, old_entries[i].key, old_entries[i].value);
        }
    }

    free(old_entries);
    return 0;
}

int font_htab_insert(font_htab *htab, uint32_t key, void *value) {
    if (value == NULL)
        return 0;

    /* Calculate the current load factor */
    if (((htab->used + 1) * 100) / htab->size > INT_HTAB_MAX_LOAD) {
        if (font_htab_resize(htab, 2 * htab->size) < 0) {
            return 0;
        }
    }

    _htab_insert(htab, key, value);

    return 1;
}

void *font_htab_find(font_htab *htab, uint32_t key) {
    unsigned int mask = htab->size - 1;
    unsigned int idx  = FNV_1a(key) & mask;

    /* Open addressing, linear probing */
    while (htab->entries[idx].key != key && htab->entries[idx].value != NULL) {
        idx = (idx + 1) & mask;
    }

    /* Found the key */
    if (htab->entries[idx].key == key) {
        return htab->entries[idx].value;
    }

    return NULL;
}
