/*
 * Copyright (C) 2009 The Android Open Source Project
 */

#ifndef _ANDROID_LOG_H
#define _ANDROID_LOG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>

/* Android log priority values, in ascending priority order */
typedef enum android_LogPriority {
    ANDROID_LOG_UNKNOWN = 0,
    ANDROID_LOG_DEFAULT,
    ANDROID_LOG_VERBOSE,
    ANDROID_LOG_DEBUG,
    ANDROID_LOG_INFO,
    ANDROID_LOG_WARN,
    ANDROID_LOG_ERROR,
    ANDROID_LOG_FATAL,
    ANDROID_LOG_SILENT,
} android_LogPriority;

/* Send a simple string to the log */
int __android_log_write(int prio, const char *tag, const char *text);

/* Send a formatted string to the log, used like printf(fmt,...) */
int __android_log_print(int prio, const char *tag, const char *fmt, ...)
    __attribute__((format(printf, 3, 4)));

/* A variant of __android_log_print() that takes a va_list */
int __android_log_vprint(int prio, const char *tag, const char *fmt, va_list ap)
    __attribute__((__format__(printf, 3, 0)));

/* Log an assertion failure and abort */
void __android_log_assert(const char *cond, const char *tag, const char *fmt, ...)
    __attribute__((__noreturn__)) __attribute__((__format__(printf, 3, 4)));

#ifdef __cplusplus
}
#endif

#endif /* _ANDROID_LOG_H */