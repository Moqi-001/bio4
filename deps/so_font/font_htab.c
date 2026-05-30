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
    font_htab *htab = malloc(sizeof(*htab));
    if (!htab)
        return NULL;

    htab->size = next_pow2(size);
    htab->used = 0;

    htab->entries = malloc(htab->size * sizeof(*htab->entries));
    memset(htab->entries, 0, htab->size * sizeof(*htab->entries));

    return htab;
}

void font_htab_free(font_htab *htab) {
    for (size_t i = 0; i < htab->size; i++) {
        if (htab->entries[i].value != NULL)
            free(htab->entries[i].value);
    }
    free(htab);
}

void font_htab_resize(font_htab *htab, size_t new_size) {
    htab_entry *old_entries;
    size_t      old_size;

    old_entries = htab->entries;
    old_size    = htab->size;

    htab->size    = new_size;
    htab->used    = 0;
    htab->entries = malloc(new_size * sizeof(*htab->entries));
    memset(htab->entries, 0, new_size * sizeof(*htab->entries));

    for (size_t i = 0; i < old_size; i++) {
        if (old_entries[i].value != NULL) {
            font_htab_insert(htab, old_entries[i].key, old_entries[i].value);
        }
    }

    free(old_entries);
}

int font_htab_insert(font_htab *htab, uint32_t key, void *value) {
    if (value == NULL)
        return 0;

    /* Calculate the current load factor */
    if (((htab->used + 1) * 100) / htab->size > INT_HTAB_MAX_LOAD) {
        font_htab_resize(htab, 2 * htab->size);
    }

    unsigned int mask = htab->size - 1;
    unsigned int idx  = FNV_1a(key) & mask;

    /* Open addressing, linear probing */
    while (htab->entries[idx].value != NULL) {
        idx = (idx + 1) & mask;
    }

    htab->entries[idx].key   = key;
    htab->entries[idx].value = value;
    htab->used++;

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
