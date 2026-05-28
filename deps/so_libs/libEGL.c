#include <so_linker/so_linker.h>
#include <so_common/so_utils.h>
#include <vitaGL.h>

// ==================== libEGL.so ====================
static so_symbol libEGL_symbols[] = {
    SO_SYMBOL_FUNC("eglGetError", &eglGetError),
    SO_SYMBOL_FUNC("eglGetDisplay", &eglGetDisplay),
    SO_SYMBOL_FUNC("eglInitialize", &eglInitialize),
    SO_SYMBOL_FUNC("eglTerminate", &eglTerminate),
    SO_SYMBOL_FUNC("eglQueryString", &eglQueryString),
    SO_SYMBOL_FUNC("eglGetConfigs", &eglGetConfigs),
    SO_SYMBOL_FUNC("eglChooseConfig", &eglChooseConfig),
    SO_SYMBOL_FUNC("eglGetConfigAttrib", &eglGetConfigAttrib),
    SO_SYMBOL_FUNC("eglCreateWindowSurface", &eglCreateWindowSurface),
    SO_SYMBOL_FUNC("eglCreatePbufferSurface", &so_ret_0),
    SO_SYMBOL_FUNC("eglCreatePixmapSurface", &so_ret_0),
    SO_SYMBOL_FUNC("eglDestroySurface", &eglDestroySurface),
    SO_SYMBOL_FUNC("eglQuerySurface", &eglQuerySurface),
    SO_SYMBOL_FUNC("eglBindAPI", &eglBindAPI),
    SO_SYMBOL_FUNC("eglQueryAPI", &eglQueryAPI),
    SO_SYMBOL_FUNC("eglWaitClient", &so_ret_0),
    SO_SYMBOL_FUNC("eglReleaseThread", &so_ret_0),
    SO_SYMBOL_FUNC("eglCreatePbufferFromClientBuffer", &so_ret_0),
    SO_SYMBOL_FUNC("eglSurfaceAttrib", &so_ret_0),
    SO_SYMBOL_FUNC("eglBindTexImage", &so_ret_0),
    SO_SYMBOL_FUNC("eglReleaseTexImage", &so_ret_0),
    SO_SYMBOL_FUNC("eglSwapInterval", &eglSwapInterval),
    SO_SYMBOL_FUNC("eglCreateContext", &eglCreateContext),
    SO_SYMBOL_FUNC("eglDestroyContext", &eglDestroyContext),
    SO_SYMBOL_FUNC("eglMakeCurrent", &eglMakeCurrent),
    SO_SYMBOL_FUNC("eglGetCurrentContext", &eglGetCurrentContext),
    SO_SYMBOL_FUNC("eglGetCurrentSurface", &so_ret_0),
    SO_SYMBOL_FUNC("eglGetCurrentDisplay", &so_ret_0),
    SO_SYMBOL_FUNC("eglQueryContext", &eglQueryContext),
    SO_SYMBOL_FUNC("eglWaitGL", &so_ret_0),
    SO_SYMBOL_FUNC("eglWaitNative", &so_ret_0),
    SO_SYMBOL_FUNC("eglSwapBuffers", &eglSwapBuffers),
    SO_SYMBOL_FUNC("eglCopyBuffers", &so_ret_0),
    SO_SYMBOL_FUNC("eglGetProcAddress", &eglGetProcAddress),
    SO_SYMBOL_FUNC("eglLockSurfaceKHR", &so_ret_0),
    SO_SYMBOL_FUNC("eglUnlockSurfaceKHR", &so_ret_0),
};

so_library so_library_EGL = {
    .soname      = "libEGL.so",
    .symbols     = libEGL_symbols,
    .num_symbols = sizeof(libEGL_symbols) / sizeof(libEGL_symbols[0]),
    .prepare     = NULL,
};
