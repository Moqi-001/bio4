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
    return JniCallNewStringUTF(FILES_PATH);
}

static so_jsymbol class_symbols[] = {
    // 静态方法
    SO_JSYMBOL_STATIC_METHOD("getFilePath", "()Ljava/lang/String;", &getFilePath),
};

static so_jclass_meta class_meta = {
    .name          = "jp/co/capcom/android/bio4_LGUplus0119/DeviceInfo",
    .super_name    = NULL,
    .symbols       = class_symbols,
    .num_symbols   = sizeof(class_symbols) / sizeof(class_symbols[0]),
    .instance_size = sizeof(DeviceInfo),
};

void DeviceInfo_class_init() {
    so_jni_register_class_from_meta(&class_meta);
}
