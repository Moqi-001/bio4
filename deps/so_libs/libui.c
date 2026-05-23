#include <so_linker/so_linker.h>
#include <so_common/so_logger.h>
#include <so_common/so_utils.h>

// ==================== libui.so ====================
// Android UI 库（显示、输入等）
static so_symbol libui_symbols[] = {
    // 需要根据实际需要添加
    SO_SYMBOL_FUNC("_ZN7android12DisplayInfoC1Ev", &so_ret_0),
};

so_library so_library_ui = {
    .soname      = "libui.so",
    .symbols     = libui_symbols,
    .num_symbols = sizeof(libui_symbols) / sizeof(libui_symbols[0]),
    .prepare     = NULL,
};
