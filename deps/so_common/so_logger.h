#ifndef SO_COMMON_LOGGER_H
#define SO_COMMON_LOGGER_H

#ifdef __cplusplus
extern "C" {
#endif

// 日志类别
#define SO_LT_UNDEF 0  // 未定义
#define SO_LT_DEBUG 1  // 调试信息
#define SO_LT_INFO  2  // 普通信息
#define SO_LT_WARN  3  // 警告信息
#define SO_LT_ERROR 4  // 错误信息
#define SO_LT_FATAL 5  // 致命错误
#define SO_LT_TEMP  6  // 临时调试

#ifdef DEBUG_SOLOADER
#define so_log(...)  so_log_printf(SO_LT_UNDEF, __VA_ARGS__)
#define so_logd(...) so_log_printf(SO_LT_DEBUG, __VA_ARGS__)
#define so_logi(...) so_log_printf(SO_LT_INFO, __VA_ARGS__)
#define so_logw(...) so_log_printf(SO_LT_WARN, __VA_ARGS__)
#else
#define so_log(...)
#define so_logd(...)
#define so_logi(...)
#define so_logw(...)
#endif

// 总是输出的日志宏 (无视 DEBUG_SOLOADER)
#define so_loge(...) so_log_printf(SO_LT_ERROR, __VA_ARGS__)  // 错误
#define so_logf(...) so_log_printf(SO_LT_FATAL, __VA_ARGS__)  // 致命错误 (会退出程序)
#define so_logt(...) so_log_printf(SO_LT_TEMP, __VA_ARGS__)   // 临时调试

void so_log_printf(int t, const char *fmt, ...)
    __attribute__((format(printf, 2, 3)));

#ifdef __cplusplus
}
#endif

#endif