#include <sys/time.h>

#include <so_jni/so_jni.h>

#include "System.h"

// static, "currentTimeMillis", "()J"
jlong JSystem_currentTimeMillis() {
    struct timeval te;
    gettimeofday(&te, NULL);
    return (jlong)(te.tv_sec * 1000LL + te.tv_usec / 1000);
}

static so_jsymbol class_symbols[] = {
    // 静态方法
    SO_JSYMBOL_STATIC_METHOD("currentTimeMillis", "()J", &JSystem_currentTimeMillis),
};

static so_jclass_meta class_meta = {
    .name          = "java/lang/System",
    .super_name    = NULL,
    .symbols       = class_symbols,
    .num_symbols   = sizeof(class_symbols) / sizeof(class_symbols[0]),
    .instance_size = 0,
};

void JSystem_class_init() {
    so_jni_register_class_from_meta(&class_meta);
}
