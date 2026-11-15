#include <stdarg.h>
#include <stdlib.h>

#include <psp2/kernel/clib.h>
#include <so_common/so_logger.h>

#include "log.h"

#define LOG_BUF_SIZE 1024

// FATAL 走 so_loge 而不是 so_logf：后者会让 so_log_printf 直接退进程，
// 而真正要退出的 __android_log_assert 自己会 abort()。
int __android_log_write(int prio, const char *tag, const char *text) {
    switch (prio) {
        case ANDROID_LOG_VERBOSE:
            so_logv("[ALOG][%s] %s", tag, text);
            break;
        case ANDROID_LOG_DEBUG:
            so_logd("[ALOG][%s] %s", tag, text);
            break;
        case ANDROID_LOG_INFO:
            so_logi("[ALOG][%s] %s", tag, text);
            break;
        case ANDROID_LOG_WARN:
            so_logw("[ALOG][%s] %s", tag, text);
            break;
        case ANDROID_LOG_ERROR:
        case ANDROID_LOG_FATAL:
            so_loge("[ALOG][%s] %s", tag, text);
            break;
        default:
            so_log("[ALOG][%s] %s", tag, text);
            break;  // 未声明/未知级别
    }

    return 0;
}

int __android_log_print(int prio, const char *tag, const char *fmt, ...) {
    va_list ap;
    char    buf[LOG_BUF_SIZE];

    va_start(ap, fmt);
    sceClibVsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);

    return __android_log_write(prio, tag, buf);
}

int __android_log_vprint(int prio, const char *tag, const char *fmt, va_list ap) {
    char buf[LOG_BUF_SIZE];

    sceClibVsnprintf(buf, sizeof(buf), fmt, ap);

    return __android_log_write(prio, tag, buf);
}

void __android_log_assert(const char *cond, const char *tag, const char *fmt, ...) {
    char buf[LOG_BUF_SIZE];

    if (fmt) {
        va_list ap;
        va_start(ap, fmt);
        sceClibVsnprintf(buf, sizeof(buf), fmt, ap);
        va_end(ap);
    } else if (cond) {
        sceClibSnprintf(buf, sizeof(buf), "Assertion failed: %s", cond);
    } else {
        sceClibSnprintf(buf, sizeof(buf), "Assertion failed");
    }

    __android_log_write(ANDROID_LOG_FATAL, tag, buf);
    abort();
}
