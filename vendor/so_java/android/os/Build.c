#include <stddef.h>

#include <so_java/java/lang/Class.h>

#include "Build.h"

struct _Build {
    uint32_t reserved;
};

JInt android_os_Build_VERSION_SDK_INT = 19;  // Android 4.4 / KitKat

static JClass sVersionClass = NULL;

static JSymbol VersionClass_symbols[] = {
    JAVA_SYMBOL_STATIC_FIELD("SDK_INT", "I", &android_os_Build_VERSION_SDK_INT),
};

static JClassMeta VersionClass_meta = {
    .name          = "android/os/Build$VERSION",
    .super_name    = NULL,
    .symbols       = VersionClass_symbols,
    .num_symbols   = sizeof(VersionClass_symbols) / sizeof(VersionClass_symbols[0]),
    .instance_size = 0,
    .dtor          = NULL,
};

static void setup_Build_VERSION(void) {
    sVersionClass = JavaCreateClass(&VersionClass_meta);
}

void setup_android_os_Build(void) {
    setup_Build_VERSION();
}
