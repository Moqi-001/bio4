#include <stdarg.h>
#include <stdatomic.h>
#include <stdbool.h>

#include <psp2/io/fcntl.h>
#include <psp2/kernel/clib.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/kernel/threadmgr.h>

#include "so_logger.h"

static SceKernelLwMutexWork _log_mutex       = {0};
static atomic_bool          _log_mutex_ready = false;

static char _log_buffer_a[2048];
static char _log_buffer_b[2048];

static inline void _log_lock(void) {
    if (!atomic_load_explicit(&_log_mutex_ready, memory_order_acquire)) {
        sceKernelCreateLwMutex(&_log_mutex, "_log_lock", 0, 0, NULL);
        atomic_store_explicit(&_log_mutex_ready, true, memory_order_release);
    }
    sceKernelLockLwMutex(&_log_mutex, 1, NULL);
}

static inline void _log_unlock(void) {
    sceKernelUnlockLwMutex(&_log_mutex, 1);
}

void so_log_printf(int t, const char *fmt, ...) {
    _log_lock();

    va_list args;
    va_start(args, fmt);
    sceClibVsnprintf(_log_buffer_a, sizeof(_log_buffer_a), fmt, args);
    va_end(args);

    const char *prefix = "";
    switch (t) {
        case SO_LT_DEBUG:
            prefix = "[DEBUG] ";
            break;
        case SO_LT_INFO:
            prefix = "[INFO] ";
            break;
        case SO_LT_WARN:
            prefix = "[WARN] ";
            break;
        case SO_LT_ERROR:
            prefix = "[ERROR] ";
            break;
        case SO_LT_FATAL:
            prefix = "[FATAL] ";
            break;
        case SO_LT_TEMP:
            prefix = "[=TEMP=] ";
            break;
        default:
            break;
    }

    sceClibSnprintf(_log_buffer_b, sizeof(_log_buffer_b), "%s%s\n", prefix, _log_buffer_a);
    sceClibPrintf("%s", _log_buffer_b);

#if defined(DEBUG_LOGFILE)
#ifndef LOG_PATH
#error "LOG_PATH must be defined when DEBUG_LOGFILE is enabled"
#endif
    SceUID fd = sceIoOpen(LOG_PATH, SCE_O_WRONLY | SCE_O_CREAT | SCE_O_APPEND, 0777);
    if (fd >= 0) {
        sceIoWrite(fd, _log_buffer_b, strlen(_log_buffer_b));
        sceIoClose(fd);
    }
#endif

    _log_unlock();

    if (t == SO_LT_FATAL) {
        sceKernelExitProcess(0);
    }
}
