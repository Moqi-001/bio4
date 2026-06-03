#include <so_common/so_logger.h>
#include <so_jni/so_jni.h>
#include <so_linker/so_linker.h>

int AndroidBitmap_lockPixels(JNIEnv *env, jobject jbitmap, void **addrPtr) {
    if (NULL == env || NULL == jbitmap) {
        return -1;
    }

    so_loge("AndroidBitmap_unlockPixels: not implemented!");
    return -1;
}

int AndroidBitmap_unlockPixels(JNIEnv *env, jobject jbitmap) {
    if (NULL == env || NULL == jbitmap) {
        return -1;
    }

    so_loge("AndroidBitmap_unlockPixels: not implemented!");
    return -1;
}

// ==================== libdvm.so ====================
static so_symbol library_symbols[] = {
    SO_SYMBOL_FUNC("AndroidBitmap_lockPixels", &AndroidBitmap_lockPixels),
    SO_SYMBOL_FUNC("AndroidBitmap_unlockPixels", &AndroidBitmap_unlockPixels),
};

so_library so_library_jnigraphics = {
    .soname      = "libjnigraphics.so",
    .symbols     = library_symbols,
    .num_symbols = sizeof(library_symbols) / sizeof(library_symbols[0]),
    .prepare     = NULL,
};
