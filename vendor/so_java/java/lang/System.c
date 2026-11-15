#include <sys/time.h>

#include "Class.h"
#include "System.h"

static JClass sClass = NULL;

JLong currentTimeMillis() {
    struct timeval te;
    gettimeofday(&te, NULL);
    return (JLong)(te.tv_sec * 1000LL + te.tv_usec / 1000);
}

static JSymbol class_symbols[] = {
    JAVA_SYMBOL_STATIC_METHOD("currentTimeMillis", "()J", &currentTimeMillis),
};

static JClassMeta class_meta = {
    .name          = "java/lang/System",
    .super_name    = NULL,
    .symbols       = class_symbols,
    .num_symbols   = sizeof(class_symbols) / sizeof(class_symbols[0]),
    .instance_size = 0,
    .dtor          = NULL,
};

void setup_java_lang_System() {
    sClass = JavaCreateClass(&class_meta);
}
