#include <so_common/so_logger.h>

#include "asset_manager_jni.h"

extern AAssetManager *AAssetManager_create();

AAssetManager *AAssetManager_fromJava(JNIEnv *env, jobject assetManager) {
    AAssetManager *mgr = (AAssetManager *)assetManager;
    if (!mgr) {
        mgr = AAssetManager_create();
    }

    so_logd("AAssetManager_fromJava(%p, %p)", mgr);
    return mgr;
}
