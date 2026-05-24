#include <sys/time.h>

#include <so_jni/so_jni.h>

#include "System.h"

// static, "currentTimeMillis", "()J"
jlong JSystem_currentTimeMillis() {
    struct timeval te;
    gettimeofday(&te, NULL);
    return (jlong)(te.tv_sec * 1000LL + te.tv_usec / 1000);
}

static so_jsymbol JSystem_symbols[] = {
    // 静态方法
    SO_JSYMBOL_STATIC_METHOD("currentTimeMillis", "()J", &JSystem_currentTimeMillis),
};

so_jclass_meta JSystem_jclass_meta = {
    .name          = "java/lang/System",
    .super_name    = NULL,
    .symbols       = JSystem_symbols,
    .num_symbols   = sizeof(JSystem_symbols) / sizeof(JSystem_symbols[0]),
    .instance_size = 0,
};
