#include <so_common/so_logger.h>
#include <so_common/so_utils.h>
#include <so_linker/so_linker.h>

// ==================== libcutils.so ====================
// Android 通用工具库

// 未实现（调用时会输出错误日志）
SO_UNIMPLEMENTED(android_get_control_socket, -1)
SO_UNIMPLEMENTED(android_reboot, -1)
SO_UNIMPLEMENTED(call_weak_function, 0)

static so_symbol libcutils_symbols[] = {
    // 内存相关
    SO_SYMBOL_FUNC("android_get_control_socket", &android_get_control_socket_soloader),
    SO_SYMBOL_FUNC("android_reboot", &android_reboot_soloader),

    // 线程相关
    SO_SYMBOL_FUNC("call_weak_function", &call_weak_function_soloader),
};

so_library libcutils_so = {
    .soname      = "libcutils.so",
    .symbols     = libcutils_symbols,
    .num_symbols = sizeof(libcutils_symbols) / sizeof(libcutils_symbols[0]),
    .prepare     = NULL,
};
