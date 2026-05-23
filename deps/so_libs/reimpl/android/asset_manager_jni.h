#ifndef ANDROID_ASSET_MANAGER_JNI_H
#define ANDROID_ASSET_MANAGER_JNI_H

#include <android/asset_manager.h>
#include <jni.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Given a Dalvik AssetManager object, obtain the corresponding native AAssetManager
 * object.  Note that the caller is responsible for obtaining and holding a VM reference
 * to the jobject to prevent its being garbage collected while the native object is
 * in use.
 */
AAssetManager* AAssetManager_fromJava(JNIEnv* env, jobject assetManager);

#ifdef __cplusplus
};
#endif

#endif      // ANDROID_ASSET_MANAGER_JNI_H