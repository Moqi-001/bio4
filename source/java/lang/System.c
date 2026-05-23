#include <sys/time.h>

#include <so_jni/so_jni.h>

// public static
jlong System_currentTimeMillis() {
    struct timeval te;
    gettimeofday(&te, NULL);
    return (jlong)(te.tv_sec * 1000LL + te.tv_usec / 1000);
}

static so_jsymbol System_symbols[] = {
    // 静态方法
    SO_JSYMBOL_STATIC_METHOD("currentTimeMillis", "()J", &System_currentTimeMillis),
};

so_jclass_meta System_jclass_meta = {
    .name          = "java/lang/System",
    .super_name    = NULL,
    .symbols       = System_symbols,
    .num_symbols   = sizeof(System_symbols) / sizeof(System_symbols[0]),
    .instance_size = 0,
};
