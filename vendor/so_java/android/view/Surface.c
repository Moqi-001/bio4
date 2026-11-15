#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <so_common/so_logger.h>
#include <so_java/java/lang/Class.h>
#include <so_java/java/lang/Object.h>

#include "Surface.h"

struct _Surface {
    int mSurface;
    int mNativeSurface;
    int mNativeObject;
};

static JClass sClass = NULL;

void _surface_ctor(Surface thiz) {
    memset(thiz, 0, sizeof(struct _Surface));
}

void _surface_dtor(Surface thiz) {
}

Surface Surface_new() {
    Surface surface = Object_new(sClass);
    if (!surface) {
        return NULL;
    }

    _surface_ctor(surface);

    return surface;
}

static JSymbol class_symbols[] = {
    // static native
    JAVA_SYMBOL_FIELD("mSurface", "I", offsetof(struct _Surface, mSurface)),
    JAVA_SYMBOL_FIELD("mNativeSurface", "I", offsetof(struct _Surface, mNativeSurface)),
    JAVA_SYMBOL_FIELD("mNativeObject", "I", offsetof(struct _Surface, mNativeObject)),
};

static JClassMeta class_meta = {
    .name          = "android/view/Surface",
    .super_name    = NULL,
    .symbols       = class_symbols,
    .num_symbols   = sizeof(class_symbols) / sizeof(class_symbols[0]),
    .instance_size = sizeof(struct _Surface),
    .dtor          = (JDtor)_surface_dtor,
};

void setup_android_view_Surface(void) {
    sClass = JavaCreateClass(&class_meta);
}
