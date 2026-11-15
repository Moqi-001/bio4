#include <stdint.h>
#include <string.h>

#include <so_common/so_logger.h>
#include <so_linker/so_linker.h>

void *dlopen_soloader(const char *filename, int flag) {
    if (!filename) {
        return NULL;
    }

    const char *soname = strrchr(filename, '/');
    if (!soname++) {
        soname = filename;
    }

    so_module *mod = so_linker_find_module(soname);
    if (!mod) {
        so_logw("[libdl] dlopen(%s,0x%x): not found!", filename, flag);
        return NULL;
    }

    so_logd("[libdl] dlopen(%s,0x%x): %p", filename, flag, mod);
    return (void *)mod;
}

void *dlsym_soloader(void *handle, const char *symbol) {
    if (!handle || !symbol) {
        return NULL;
    }

    so_module *mod = (so_module *)handle;
    void      *sym = so_module_find_export(mod, symbol);

    if (!sym) {
        const char *soname = so_module_get_name(mod);
        so_logw("[libdl] %s: '%s' not found!", soname ? soname : "?", symbol);
        return NULL;
    }

    so_logd("[libdl] dlsym(%p,%s): %p", handle, symbol, sym);
    return sym;
}

void dlclose_soloader(void *handle) {
    so_logd("[libdl] dlclose(%p): ignored", handle);
}

char *dlerror_soloader(void) {
    so_logd("[libdl] dlerror(): ignored");
    return NULL;
}
