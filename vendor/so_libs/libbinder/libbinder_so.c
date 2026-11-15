#include <so_common/so_logger.h>
#include <so_common/so_utils.h>
#include <so_linker/so_linker.h>

// ==================== libbinder.so ====================
// 未实现（调用时会输出错误日志）
SO_UNIMPLEMENTED_LABELED(android_ProcessState_selfWithDriver, "_ZN7android11ProcessState14selfWithDriverEi", 0)
SO_UNIMPLEMENTED_LABELED(android_ProcessState_startThreadPool, "_ZN7android11ProcessState11startThreadPoolEv", 0)
SO_UNIMPLEMENTED_LABELED(android_IBinder_pingBinder, "_ZN7android7IBinder14pingBinderEv", 0)

so_symbol libbinder_symbols[] = {
    // ProcessState
    SO_SYMBOL_FUNC("_ZN7android11ProcessState14selfWithDriverEi", &android_ProcessState_selfWithDriver_soloader),
    SO_SYMBOL_FUNC("_ZN7android11ProcessState11startThreadPoolEv", &android_ProcessState_startThreadPool_soloader),

    // IBinder
    SO_SYMBOL_FUNC("_ZN7android7IBinder14pingBinderEv", &android_IBinder_pingBinder_soloader),
};

so_library libbinder_so = {
    .soname      = "libbinder.so",
    .symbols     = libbinder_symbols,
    .num_symbols = sizeof(libbinder_symbols) / sizeof(libbinder_symbols[0]),
    .prepare     = NULL,
};
