#include <stdint.h>
#include <string.h>

#include <so_common/so_logger.h>
#include <so_linker/so_linker.h>

#define TRAP_MAX        256
#define TRAP_SIZE       16
#define TRAP_SYMNAME_SZ 128
#define TRAP_SONAME_SZ  64

static uint8_t *s_trap_base;
static char     s_trap_names[TRAP_MAX][TRAP_SYMNAME_SZ];
static char     s_trap_sonames[TRAP_MAX][TRAP_SONAME_SZ];
static uint8_t  s_trap_reported[TRAP_MAX];
static int      s_trap_count;

static int missing_symbol_trap(uint32_t index) {
    if (index < TRAP_MAX) {
        so_loge("[libdl] %s: missing symbol '%s' is called!", s_trap_sonames[index], s_trap_names[index]);
    }

    return 0;
}

static void *create_missing_symbol_trampoline(const char *soname, const char *name) {
    if (!s_trap_base) {
        s_trap_base = so_mmap(TRAP_MAX * TRAP_SIZE);
        if (!s_trap_base) {
            so_loge("[libdl] failed to allocate trap stub memory");
            return NULL;
        }
    }

    if (s_trap_count >= TRAP_MAX) {
        so_loge("[libdl] trap stub table is full (%d entries)", TRAP_MAX);
        return NULL;
    }

    int       index = s_trap_count++;
    uint32_t *trap  = (uint32_t *)(s_trap_base + index * TRAP_SIZE);

    strncpy(s_trap_names[index], name, TRAP_SYMNAME_SZ - 1);
    strncpy(s_trap_sonames[index], soname ? soname : "?", TRAP_SONAME_SZ - 1);

    trap[0] = 0xE59F0000;  // ldr r0, [pc, #0] -> index
    trap[1] = 0xE59FF000;  // ldr pc, [pc, #0] -> trap handler
    trap[2] = (uint32_t)index;
    trap[3] = (uint32_t)&missing_symbol_trap;

    so_cacheflush(trap, TRAP_SIZE);
    return trap;
}

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
        return create_missing_symbol_trampoline(soname, symbol);
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
