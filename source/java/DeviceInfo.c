#include <stddef.h>
#include <stdint.h>

#include <so_jni/so_jni.h>

#include "java.h"

typedef struct DeviceInfo {
    uint32_t reserved;
} DeviceInfo;

#define FILES_PATH DATA_PATH

// static, "getFilePath", "()Ljava/lang/String;"
static jstring getFilePath() {
    return JniNewStringUTF(FILES_PATH);
}

static jsymbol class_symbols[] = {
    // 静态方法
    JNI_SYMBOL_STATIC_METHOD("getFilePath", "()Ljava/lang/String;", &getFilePath),
};

static jclassMeta class_meta = {
    .name          = "jp/co/capcom/android/bio4_LGUplus0119/DeviceInfo",
    .super_name    = NULL,
    .symbols       = class_symbols,
    .num_symbols   = sizeof(class_symbols) / sizeof(class_symbols[0]),
    .instance_size = sizeof(DeviceInfo),
};

void DeviceInfo_class_init() {
    JniCreateClass(&class_meta);
}
