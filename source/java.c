#include <stddef.h>

#include <so_jni/so_jni.h>

#include "java/lang/System.h"

#include "java/AssetFileInfo.h"
#include "java/Bio4PreActivity.h"
#include "java/DeviceInfo.h"
#include "java/TextBitmap.h"

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
    so_jni_register_class_from_meta(&JSystem_jclass_meta);
    so_jni_register_class_from_meta(&AssetFileInfo_jclass_meta);
    so_jni_register_class_from_meta(&DeviceInfo_jclass_meta);
    so_jni_register_class_from_meta(&TextBitmap_jclass_meta);

    Bio4PreActivity_init();

    return 0;
}
