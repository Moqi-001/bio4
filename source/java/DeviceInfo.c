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

static so_jsymbol DeviceInfo_symbols[] = {
    // 静态方法
    SO_JSYMBOL_STATIC_METHOD("getFilePath", "()Ljava/lang/String;", &getFilePath),
};

so_jclass_meta DeviceInfo_jclass_meta = {
    .name          = "jp/co/capcom/android/bio4_LGUplus0119/DeviceInfo",
    .super_name    = NULL,
    .symbols       = DeviceInfo_symbols,
    .num_symbols   = sizeof(DeviceInfo_symbols) / sizeof(DeviceInfo_symbols[0]),
    .instance_size = sizeof(DeviceInfo),
};
