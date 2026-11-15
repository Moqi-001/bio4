#include <so_common/so_logger.h>

#include "native_window.h"
#include "native_window_jni.h"

#include "../libcutils/atomic.h"

struct ANativeWindow {
    int32_t ref_count;
    int32_t width;
    int32_t height;
    int32_t format;
};

ANativeWindow g_NativeWindow = {
    .ref_count = 0,
    .width     = 960,
    .height    = 544,
    .format    = WINDOW_FORMAT_RGBA_8888,
};

ANativeWindow *ANativeWindow_fromSurface(JNIEnv *env, jobject surface) {
    ANativeWindow *window = &g_NativeWindow;
    so_logd("ANativeWindow_fromSurface(%p,%p): %p", env, surface, window);
    return window;
}

void ANativeWindow_acquire(ANativeWindow *window) {
    android_atomic_inc(&window->ref_count);
    so_logd("ANativeWindow_acquire(%p)", window);
}

void ANativeWindow_release(ANativeWindow *window) {
    android_atomic_dec(&window->ref_count);
    so_logd("ANativeWindow_release(%p)", window);
}

int32_t ANativeWindow_getWidth(ANativeWindow *window) {
    so_logd("ANativeWindow_getWidth(%p): %d", window, window->width);
    return window->width;
}

int32_t ANativeWindow_getHeight(ANativeWindow *window) {
    so_logd("ANativeWindow_getHeight(%p): %d", window, window->height);
    return window->height;
}

int32_t ANativeWindow_getFormat(ANativeWindow *window) {
    so_logd("ANativeWindow_getFormat(%p): %d", window, window->format);
    return window->format;
}

int32_t ANativeWindow_setBuffersGeometry(ANativeWindow *window, int32_t width,
                                         int32_t height, int32_t format) {
    window->width  = width > 0 ? width : 960;
    window->height = height > 0 ? height : 544;
    window->format = format > 0 ? format : WINDOW_FORMAT_RGBA_8888;
    so_logd("ANativeWindow_setBuffersGeometry(%p,%d,%d,%d): %d", window, width, height, format);
    return 0;
}

int32_t ANativeWindow_lock(ANativeWindow *window, ANativeWindow_Buffer *outBuffer,
                           ARect *inOutDirtyBounds) {
    int32_t result = 0;
    so_logd("ANativeWindow_lock(%p,%p,%p): %d", window, outBuffer, inOutDirtyBounds, result);
    return result;
}

int32_t ANativeWindow_unlockAndPost(ANativeWindow *window) {
    int32_t result = 0;
    so_logd("ANativeWindow_unlockAndPost(%p): %d", window, result);
    return result;
}
