#include <stdarg.h>
#include <stdatomic.h>

#include <psp2/io/fcntl.h>
#include <psp2/kernel/clib.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/kernel/threadmgr.h>

#include "so_logger.h"

static SceKernelLwMutexWork _log_mutex = {0};
static volatile int         _log_once  = 0;

static char _log_buffer_a[2048];
static char _log_buffer_b[2048];

static int _log_init() {
    if (atomic_load(&_log_once) == 2) {
        return 0;
    }

    int expected = 0;
    if (atomic_compare_exchange_strong(&_log_once, &expected, 1)) {
        sceKernelCreateLwMutex(&_log_mutex, "_log_lock", 0, 0, NULL);
        atomic_store(&_log_once, 2);
        return 0;
    }

    while (atomic_load(&_log_once) != 2) {
        sceKernelDelayThread(1000);
    }

    return 0;
}

#define LOGGER_INIT   _log_init();
#define LOGGER_LOCK   sceKernelLockLwMutex(&_log_mutex, 1, NULL);
#define LOGGER_UNLOCK sceKernelUnlockLwMutex(&_log_mutex, 1);

void so_log_printf(int t, const char *fmt, ...) {
    LOGGER_INIT
    LOGGER_LOCK

    va_list args;
    va_start(args, fmt);
    sceClibVsnprintf(_log_buffer_a, sizeof(_log_buffer_a), fmt, args);
    va_end(args);

    const char *prefix;
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
        case SO_LT_ALWAYS:
            prefix = "[LOG] ";
            break;
        default:
            prefix = "";
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

    if (t == SO_LT_FATAL) {
        sceKernelDelayThread(100000);
        sceKernelExitProcess(0);
    }

    LOGGER_UNLOCK
}
