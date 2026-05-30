#ifndef INT_HTAB_H
#define INT_HTAB_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef struct font_htab font_htab;

font_htab *font_htab_new(size_t size);
void       font_htab_free(font_htab *htab);
int        font_htab_insert(font_htab *htab, uint32_t key, void *value);
void      *font_htab_find(font_htab *htab, uint32_t key);

#ifdef __cplusplus
}
#endif

#endif
