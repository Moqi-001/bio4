#include <string.h>

#include <so_linker/so_linker.h>
#include <so_common/so_logger.h>
#include <so_common/so_utils.h>

void *dlopen_soloader(const char *filename, int flag) {
    if (!filename) return NULL;

    const char *soname = strrchr(filename, '/');
    if (!soname) soname = filename;

    so_module *mod = so_linker_find_module(soname);

    so_logd("[libdl] dlopen(%s,0x%x): %p", filename, flag, mod);
    return (void *)mod;
}

void *dlsym_soloader(void *handle, const char *symbol) {
    if (!handle || !symbol) return NULL;

    so_module *mod = (so_module *)handle;
    void      *sym = so_module_find_symbol(mod, symbol);

    if (!sym) {
        so_logw("[libdl] dlsym(%p,%s): not found!", handle, symbol, sym);
        return NULL;
    }

    so_logd("[libdl] dlsym(%p,%s): %p", handle, symbol, sym);
    return sym;
}

void dlclose_soloader(void *handle) {
    so_logd("[libdl] dlclose(%p): ignored", handle);
}

char *dlerror(void) {
    so_logd("[libdl] dlerror(): ignored");
    return NULL;
}

// ==================== libdl.so ====================
static so_symbol libdl_symbols[] = {
    SO_SYMBOL_FUNC("dlopen", &dlopen_soloader),
    SO_SYMBOL_FUNC("dlsym", &dlsym_soloader),
    SO_SYMBOL_FUNC("dlclose", &dlclose_soloader),
    SO_SYMBOL_FUNC("dlerror", &dlclose_soloader),
};

so_library so_library_dl = {
    .soname      = "libdl.so",
    .symbols     = libdl_symbols,
    .num_symbols = sizeof(libdl_symbols) / sizeof(libdl_symbols[0]),
    .prepare     = NULL,
};
