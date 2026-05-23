#include <so_linker/so_linker.h>
#include <so_common/so_logger.h>
#include <so_common/so_utils.h>

#include "reimpl/android/asset_manager.h"
#include "reimpl/android/asset_manager_jni.h"

// ==================== libandroid.so ====================
so_symbol libandroid_symbols[] = {
    // Asset Manager JNI
    SO_SYMBOL_FUNC("AAssetManager_fromJava", &AAssetManager_fromJava),

    // Asset Manager
    SO_SYMBOL_FUNC("AAssetManager_openDir", &AAssetManager_openDir),
    SO_SYMBOL_FUNC("AAssetManager_open", &AAssetManager_open),
    SO_SYMBOL_FUNC("AAssetDir_getNextFileName", &AAssetDir_getNextFileName),
    SO_SYMBOL_FUNC("AAssetDir_rewind", &AAssetDir_rewind),
    SO_SYMBOL_FUNC("AAssetDir_close", &AAssetDir_close),
    SO_SYMBOL_FUNC("AAsset_read", &AAsset_read),
    SO_SYMBOL_FUNC("AAsset_seek", &AAsset_seek),
    SO_SYMBOL_FUNC("AAsset_seek64", &AAsset_seek64),
    SO_SYMBOL_FUNC("AAsset_close", &AAsset_close),
    SO_SYMBOL_FUNC("AAsset_getBuffer", &AAsset_getBuffer),
    SO_SYMBOL_FUNC("AAsset_getLength", &AAsset_getLength),
    SO_SYMBOL_FUNC("AAsset_getLength64", &AAsset_getLength64),
    SO_SYMBOL_FUNC("AAsset_getRemainingLength", &AAsset_getRemainingLength),
    SO_SYMBOL_FUNC("AAsset_getRemainingLength64", &AAsset_getRemainingLength64),
    SO_SYMBOL_FUNC("AAsset_openFileDescriptor", &AAsset_openFileDescriptor),
    SO_SYMBOL_FUNC("AAsset_openFileDescriptor64", &AAsset_openFileDescriptor64),
    SO_SYMBOL_FUNC("AAsset_isAllocated", &AAsset_isAllocated),
};

so_library so_library_android = {
    .soname      = "libandroid.so",
    .symbols     = libandroid_symbols,
    .num_symbols = sizeof(libandroid_symbols) / sizeof(libandroid_symbols[0]),
    .prepare     = NULL,
};
