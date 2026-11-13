#include <so_linker/so_linker.h>

#include "dl.h"

// ==================== libdl.so ====================
static so_symbol libdl_symbols[] = {
    SO_SYMBOL_FUNC("dlopen", &dlopen_soloader),
    SO_SYMBOL_FUNC("dlsym", &dlsym_soloader),
    SO_SYMBOL_FUNC("dlclose", &dlclose_soloader),
    SO_SYMBOL_FUNC("dlerror", &dlerror_soloader),
};

so_library libdl_so = {
    .soname      = "libdl.so",
    .symbols     = libdl_symbols,
    .num_symbols = sizeof(libdl_symbols) / sizeof(libdl_symbols[0]),
    .prepare     = NULL,
};
