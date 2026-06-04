#include <stddef.h>
#include <stdint.h>

#include <so_jni/so_jni.h>

#include "java.h"

typedef struct DeviceInfo {
    uint32_t reserved;
} DeviceInfo;

#define FILES_PATH         DATA_PATH
#define DOWNLOAD_DATA_PATH DATA_PATH "assets/"

// static, "getFilePath", "()Ljava/lang/String;"
static jstring getFilePath() {
    return JniNewStringUTF(FILES_PATH);
}

// static, "getdownloadDataPath","()Ljava/lang/String;""
static jstring getdownloadDataPath() {
    return JniNewStringUTF(DOWNLOAD_DATA_PATH);
}

static jsymbol class_symbols[] = {
    // 静态方法
    JNI_SYMBOL_STATIC_METHOD("getFilePath", "()Ljava/lang/String;", &getFilePath),
    JNI_SYMBOL_STATIC_METHOD("getdownloadDataPath", "()Ljava/lang/String;", &getdownloadDataPath),
};

static jclassMeta class_meta = {
    .name          = "jjp/co/capcom/android/googleplay/Evil4/DeviceInfo",
    .super_name    = NULL,
    .symbols       = class_symbols,
    .num_symbols   = sizeof(class_symbols) / sizeof(class_symbols[0]),
    .instance_size = sizeof(DeviceInfo),
};

void DeviceInfo_class_init() {
    JniCreateClass(&class_meta);
}
