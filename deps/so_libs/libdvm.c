#include <so_jni/so_jni.h>
#include <so_linker/so_linker.h>

// ==================== libdvm.so ====================
static so_symbol libdvm_symbols[] = {
    SO_SYMBOL_FUNC("JNI_GetDefaultJavaVMInitArgs", &JNI_GetDefaultJavaVMInitArgs),
    SO_SYMBOL_FUNC("JNI_CreateJavaVM", &JNI_CreateJavaVM),
    SO_SYMBOL_FUNC("JNI_GetCreatedJavaVMs", &JNI_GetCreatedJavaVMs),
};

so_library so_library_dvm = {
    .soname      = "libdvm.so",
    .symbols     = libdvm_symbols,
    .num_symbols = sizeof(libdvm_symbols) / sizeof(libdvm_symbols[0]),
    .prepare     = NULL,
};
