#include <so_jni/so_jni.h>
#include <so_linker/so_linker.h>

#include "JNIHelp.h"

// ==================== libnativehelper.so ====================
// 安卓 JNI 辅助库
static so_symbol libnativehelper_symbols[] = {
    // JNIHelp
    SO_SYMBOL_FUNC("jniRegisterNativeMethods", &jniRegisterNativeMethods),
    SO_SYMBOL_FUNC("jniThrowException", &jniThrowException),

    // JniInvocation
    SO_SYMBOL_FUNC("JNI_GetDefaultJavaVMInitArgs", &JNI_GetDefaultJavaVMInitArgs),
    SO_SYMBOL_FUNC("JNI_CreateJavaVM", &JNI_CreateJavaVM),
    SO_SYMBOL_FUNC("JNI_GetCreatedJavaVMs", &JNI_GetCreatedJavaVMs),
};

so_library libnativehelper_so = {
    .soname      = "libnativehelper.so",
    .symbols     = libnativehelper_symbols,
    .num_symbols = sizeof(libnativehelper_symbols) / sizeof(libnativehelper_symbols[0]),
    .prepare     = NULL,
};
