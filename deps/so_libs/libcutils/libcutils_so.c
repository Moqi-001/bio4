#include <so_common/so_logger.h>
#include <so_common/so_utils.h>
#include <so_linker/so_linker.h>

// ==================== libcutils.so ====================
// Android 通用工具库
static so_symbol libcutils_symbols[] = {
    // 内存相关
    SO_SYMBOL_FUNC("android_get_control_socket", &so_ret_0),
    SO_SYMBOL_FUNC("android_reboot", &so_ret_0),

    // 线程相关
    SO_SYMBOL_FUNC("call_weak_function", &so_ret_0),
};

so_library libcutils_so = {
    .soname      = "libcutils.so",
    .symbols     = libcutils_symbols,
    .num_symbols = sizeof(libcutils_symbols) / sizeof(libcutils_symbols[0]),
    .prepare     = NULL,
};
