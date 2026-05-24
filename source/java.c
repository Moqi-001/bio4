#include <stddef.h>

#include <so_jni/so_jni.h>

#include "java/lang/System.h"

#include "java/AssetFileInfo.h"
#include "java/Bio4PreActivity.h"
#include "java/DeviceInfo.h"
#include "java/StereoViewAdapter.h"
#include "java/TextBitmap.h"
#include "java/FileManager.h"

jarray JniNewArray(jsize length, int type) {
    // 创建实例
    so_jarray *instance = so_jarray_new(length, type);
    if (!instance) {
        return NULL;
    }
    // 创建 jobject 封装（jni用）
    jarray array = so_jobject_new(NULL, instance);
    if (!array) {
        so_jinstance_release(instance);
        return NULL;
    }

    return array;
}

int soloader_java_init() {
    JSystem_class_init();

    AssetFileInfo_class_init();
    Bio4PreActivity_class_init();
    DeviceInfo_class_init();
    FileManager_class_init();
    StereoViewAdapter_class_init();
    TextBitmap_class_init();

    return 0;
}
