#include <so_common/so_logger.h>
#include <so_common/so_utils.h>
#include <so_linker/so_linker.h>

// ==================== libbinder.so ====================
so_symbol libbinder_symbols[] = {
    // ProcessState
    SO_SYMBOL_FUNC("_ZN7android11ProcessState14selfWithDriverEi", &so_ret_0),
    SO_SYMBOL_FUNC("_ZN7android11ProcessState11startThreadPoolEv", &so_ret_0),

    // IBinder
    SO_SYMBOL_FUNC("_ZN7android7IBinder14pingBinderEv", &so_ret_0),
};

so_library libbinder_so = {
    .soname      = "libbinder.so",
    .symbols     = libbinder_symbols,
    .num_symbols = sizeof(libbinder_symbols) / sizeof(libbinder_symbols[0]),
    .prepare     = NULL,
};
