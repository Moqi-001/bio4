
#include <so_jni/so_jni.h>

#include "java/AssetFileInfo.h"
#include "java/Bio4PreActivity.h"
#include "java/lang/System.h"

int soloader_java_init() {
    so_jni_register_class_from_meta(&System_jclass_meta);
    so_jni_register_class_from_meta(&AssetFileInfo_jclass_meta);
    Bio4PreActivity_init();

    return 0;
}
