#include <so_linker/so_linker.h>
#include <so_common/so_logger.h>
#include <so_common/so_utils.h>

#include "reimpl/android/log.h"

// ==================== liblog.so ====================
// Android 日志库
static so_symbol liblog_symbols[] = {
    SO_SYMBOL_FUNC("__android_log_print", &__android_log_print),
    SO_SYMBOL_FUNC("__android_log_vprint", &__android_log_vprint),
    SO_SYMBOL_FUNC("__android_log_write", &__android_log_write),
    SO_SYMBOL_FUNC("__android_log_assert", &__android_log_assert),
};

so_library so_library_log = {
    .soname      = "liblog.so",
    .symbols     = liblog_symbols,
    .num_symbols = sizeof(liblog_symbols) / sizeof(liblog_symbols[0]),
    .prepare     = NULL,
};
