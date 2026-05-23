#include <so_linker/so_linker.h>
#include <so_common/so_logger.h>
#include <so_common/so_utils.h>

#include "reimpl/android/nativehelper.h"

// ==================== libnativehelper.so ====================
// 安卓 JNI 辅助库
static so_symbol libnativehelper_symbols[] = {
    // 常见的 JNI 辅助函数
    SO_SYMBOL_FUNC("jniRegisterNativeMethods", &jniRegisterNativeMethods),
    SO_SYMBOL_FUNC("jniThrowException", &jniThrowException),
};

so_library so_library_nativehelper = {
    .soname      = "libnativehelper.so",
    .symbols     = libnativehelper_symbols,
    .num_symbols = sizeof(libnativehelper_symbols) / sizeof(libnativehelper_symbols[0]),
    .prepare     = NULL,
};
