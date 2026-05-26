#include <stdint.h>

#include "so_internal.h"
#include "so_linker.h"

static so_context *_linker_context = NULL;

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

int so_linker_unregister_module(so_module *mod) {
    return so_context_remove_module(_linker_context, mod);
}
