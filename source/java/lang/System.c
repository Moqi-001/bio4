#include <sys/time.h>

#include <so_jni/so_jni.h>

#include "System.h"

// static, "currentTimeMillis", "()J"
jlong JSystem_currentTimeMillis() {
    struct timeval te;
    gettimeofday(&te, NULL);
    return (jlong)(te.tv_sec * 1000LL + te.tv_usec / 1000);
}

static jsymbol class_symbols[] = {
    // 静态方法
    JNI_SYMBOL_STATIC_METHOD("currentTimeMillis", "()J", &JSystem_currentTimeMillis),
};

static jclassMeta class_meta = {
    .name          = "java/lang/System",
    .super_name    = NULL,
    .symbols       = class_symbols,
    .num_symbols   = sizeof(class_symbols) / sizeof(class_symbols[0]),
    .instance_size = 0,
};

void JSystem_class_init() {
    JniCreateClass(&class_meta);
}
