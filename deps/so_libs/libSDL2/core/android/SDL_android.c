
#define __ANDROID__

#include <so_common/so_logger.h>
#include <so_jni/so_jni.h>
#include <so_libs/libandroid/asset_manager.h>

#include <SDL2/SDL.h>

static AAssetManager *asset_manager = NULL;

static void Internal_Android_Create_AssetManager() {
    asset_manager = AAssetManager_create();
}

JNIEnv *Android_JNI_GetEnv(void) {
    return &_jni_env;
}

int Android_JNI_FileOpen(SDL_RWops  *ctx,
                         const char *fileName, const char *mode) {
    AAsset *asset               = NULL;
    ctx->hidden.androidio.asset = NULL;

    if (asset_manager == NULL) {
        Internal_Android_Create_AssetManager();
    }

    if (asset_manager == NULL) {
        return -1;
    }

    asset = AAssetManager_open(asset_manager, fileName, AASSET_MODE_UNKNOWN);
    if (asset == NULL) {
        return -1;
    }

    ctx->hidden.androidio.asset = (void *)asset;
    return 0;
}

size_t Android_JNI_FileRead(SDL_RWops *ctx, void *buffer,
                            size_t size, size_t maxnum) {
    size_t  result;
    AAsset *asset = (AAsset *)ctx->hidden.androidio.asset;
    result        = AAsset_read(asset, buffer, size * maxnum);

    if (result > 0) {
        /* Number of chuncks */
        return (result / size);
    } else {
        /* Error or EOF */
        return result;
    }
}

size_t Android_JNI_FileWrite(SDL_RWops *ctx, const void *buffer,
                             size_t size, size_t num) {
    SDL_SetError("Cannot write to Android package filesystem");
    return 0;
}

Sint64 Android_JNI_FileSize(SDL_RWops *ctx) {
    off64_t result;
    AAsset *asset = (AAsset *)ctx->hidden.androidio.asset;
    result        = AAsset_getLength64(asset);
    return result;
}

Sint64 Android_JNI_FileSeek(SDL_RWops *ctx, Sint64 offset, int whence) {
    off64_t result;
    AAsset *asset = (AAsset *)ctx->hidden.androidio.asset;
    result        = AAsset_seek64(asset, offset, whence);
    return result;
}

int Android_JNI_FileClose(SDL_RWops *ctx) {
    AAsset *asset = (AAsset *)ctx->hidden.androidio.asset;
    AAsset_close(asset);
    return 0;
}

void SDL_Android_Init(JNIEnv *mEnv, jclass cls) {
    so_logd("SDL_Android_Init(%p,%p)", mEnv, cls);
}
