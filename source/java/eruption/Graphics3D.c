#include <string.h>

#include <so_common/so_logger.h>
#include <so_jni/so_jni.h>
#include <vitaGL.h>

#include "java.h"

// static, "surfaceCreate", "(II)Ljavax/microedition/khronos/egl/EGLSurface;"
jobject surfaceCreate(jargs *args) {  // (int width, int height)
    jint width  = JARG(args, jint);
    jint height = JARG(args, jint);

    jobject result = JniNewStringUTF("EGLSurface");
    so_logd("[Graphics3D] surfaceCreate(%d,%d): %p", width, height, result);
    return result;
}

// static, "surfaceDestroy", "(Ljavax/microedition/khronos/egl/EGLSurface;)V"
void surfaceDestroy(jargs *args) {  // (EGLSurface surface)
    EGLSurface surface = JARG(args, jobject);

    so_logd("[Graphics3D] surfaceDestroy(%p)", surface);
}

// static, "surfaceChangeDefault", "()V"
void surfaceChangeDefault() {
    so_logd("[Graphics3D] surfaceChangeDefault()");
}

// static, "surfaceChange", "(Ljavax/microedition/khronos/egl/EGLSurface;)V"
void surfaceChange(jargs *args) {  // (EGLSurface surface)
    jobject surface = JARG(args, jobject);

    so_logd("[Graphics3D] surfaceChange(%p)", surface);
}

// static, "getDpyFormat", "()I"
jint getDpyFormat() {
    jint format = 1;
    so_logd("[Graphics3D] getDpyFormat(): 0x%x", format);
    return format;
}

static jsymbol class_symbols[] = {
    // 静态方法
    JNI_SYMBOL_STATIC_METHOD("surfaceCreate", "(II)Ljavax/microedition/khronos/egl/EGLSurface;", &surfaceCreate),
    JNI_SYMBOL_STATIC_METHOD("surfaceDestroy", "(Ljavax/microedition/khronos/egl/EGLSurface;)V", &surfaceDestroy),
    JNI_SYMBOL_STATIC_METHOD("surfaceChangeDefault", "()V", &surfaceChangeDefault),
    JNI_SYMBOL_STATIC_METHOD("surfaceChange", "(Ljavax/microedition/khronos/egl/EGLSurface;)V", &surfaceChange),
    JNI_SYMBOL_STATIC_METHOD("getDpyFormat", "()I", &getDpyFormat),
};

static jclassMeta class_meta = {
    .name          = "com/mascotcapsule/eruption/android/Graphics3D",
    .super_name    = NULL,
    .symbols       = class_symbols,
    .num_symbols   = sizeof(class_symbols) / sizeof(class_symbols[0]),
    .instance_size = 0,
};

void Graphics3D_class_init() {
    JniCreateClass(&class_meta);
}
static int i = 0x81540000;