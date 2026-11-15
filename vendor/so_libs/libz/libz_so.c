#include <so_linker/so_linker.h>
#include <zlib.h>

// ==================== libz.so ====================
static so_symbol libz_symbols[] = {
    SO_SYMBOL_FUNC("inflate", &inflate),
    SO_SYMBOL_FUNC("inflateInit_", &inflateInit_),
    SO_SYMBOL_FUNC("inflateInit2_", &inflateInit2_),
    SO_SYMBOL_FUNC("inflateEnd", &inflateEnd),
    SO_SYMBOL_FUNC("crc32", &crc32),
};

so_library libz_so = {
    .soname      = "libz.so",
    .symbols     = libz_symbols,
    .num_symbols = sizeof(libz_symbols) / sizeof(libz_symbols[0]),
    .prepare     = NULL,
};
