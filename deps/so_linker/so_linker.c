#include <stdint.h>

#include "so_internal.h"
#include "so_linker.h"

static so_context *_linker_context = NULL;

uint32_t so_sysv_hash(const char *name) {
    uint32_t       h = 0, g;
    const uint8_t *p = (const uint8_t *)name;
    while (*p) {
        h = (h << 4) + *p++;
        if ((g = (h & 0xf0000000)) != 0)
            h ^= g >> 24;
        h &= 0x0fffffff;
    }
    return h;
}

int so_linker_init() {
    _linker_context = so_context_create();
    if (!_linker_context) {
        return -1;
    }

    return 0;
}

void so_linker_fini() {
    if (_linker_context) {
        so_context_destroy(_linker_context);
        _linker_context = NULL;
    }
}

so_context *so_linker_get_context() {
    return _linker_context;
}

so_module *so_linker_find_module(const char *soname) {
    return so_context_find_module(_linker_context, soname);
}

int so_linker_register_module(so_module *mod) {
    return so_context_add_module(_linker_context, mod);
}

int so_linker_register_module_take(so_module *mod) {
    return so_context_add_module_take(_linker_context, mod);
}

int so_linker_register_module_from_library(so_library *lib) {
    so_module *mod = so_module_create(lib);
    if (!mod) {
        return -1;
    }

    if (so_context_add_module_take(_linker_context, mod) < 0) {
        so_module_close(mod);
        return -1;
    }

    return 0;
}

int so_linker_unregister_module(so_module *mod) {
    return so_context_remove_module(_linker_context, mod);
}
