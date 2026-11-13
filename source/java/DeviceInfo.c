#include <stddef.h>
#include <stdint.h>

#include <so_jni/so_jni.h>

struct _DeviceInfo {
    uint32_t reserved;
};
typedef struct _DeviceInfo *DeviceInfo;

#define FILES_PATH DATA_PATH

static JClass  sClass    = NULL;
static JString sFilePath = NULL;

static JString getFilePath() {
    return Object_retain(sFilePath);
}

static JSymbol class_symbols[] = {
    JAVA_SYMBOL_STATIC_METHOD("getFilePath", "()Ljava/lang/String;", &getFilePath),
};

static JClassMeta class_meta = {
    .name          = "jp/co/capcom/android/bio4_LGUplus0119/DeviceInfo",
    .super_name    = NULL,
    .symbols       = class_symbols,
    .num_symbols   = sizeof(class_symbols) / sizeof(class_symbols[0]),
    .instance_size = sizeof(struct _DeviceInfo),
};

void setup_DeviceInfo() {
    sClass    = JavaCreateClass(&class_meta);
    sFilePath = String_newUTF(FILES_PATH);
}
