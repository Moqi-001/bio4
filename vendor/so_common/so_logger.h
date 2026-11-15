#ifndef SO_COMMON_LOGGER_H
#define SO_COMMON_LOGGER_H

#ifdef __cplusplus
extern "C" {
#endif

// 日志级别（数值越大越重要）
#define SO_LT_UNDEF   0  // 未定义
#define SO_LT_VERBOSE 1  // 逐次探测：malloc、着色器编译之类的细粒度追踪
#define SO_LT_DEBUG   2  // 调试信息
#define SO_LT_INFO    3  // 普通信息
#define SO_LT_WARN    4  // 警告信息
#define SO_LT_ERROR   5  // 错误信息
#define SO_LT_FATAL   6  // 致命错误 (会退出程序)
#define SO_LT_ALWAYS  7  // 总是输出

// 编译期阈值：低于它的日志宏直接编成空，等于「本次构建最少输出到哪一级」。
//   -DSO_LOG_LEVEL=1  VERBOSE  连逐次探测都输出（排查 malloc/着色器时用）
//   -DSO_LOG_LEVEL=2  DEBUG    等于原来的 -DDEBUG_SOLOADER
//   -DSO_LOG_LEVEL=5  ERROR    发布构建，只剩 so_loge / so_loga
#ifndef SO_LOG_LEVEL
#ifdef DEBUG_SOLOADER
#define SO_LOG_LEVEL SO_LT_DEBUG
#else
#define SO_LOG_LEVEL SO_LT_ERROR
#endif
#endif

#if SO_LOG_LEVEL <= SO_LT_VERBOSE
#define so_logv(...) so_log_printf(SO_LT_VERBOSE, __VA_ARGS__)
#else
#define so_logv(...)
#endif

#if SO_LOG_LEVEL <= SO_LT_DEBUG
#define so_log(...)  so_log_printf(SO_LT_UNDEF, __VA_ARGS__)
#define so_logd(...) so_log_printf(SO_LT_DEBUG, __VA_ARGS__)
#else
#define so_log(...)
#define so_logd(...)
#endif

#if SO_LOG_LEVEL <= SO_LT_INFO
#define so_logi(...) so_log_printf(SO_LT_INFO, __VA_ARGS__)
#else
#define so_logi(...)
#endif

#if SO_LOG_LEVEL <= SO_LT_WARN
#define so_logw(...) so_log_printf(SO_LT_WARN, __VA_ARGS__)
#else
#define so_logw(...)
#endif

// 无视阈值的日志宏
#define so_loge(...) so_log_printf(SO_LT_ERROR, __VA_ARGS__)   // 错误
#define so_logf(...) so_log_printf(SO_LT_FATAL, __VA_ARGS__)   // 致命错误 (会退出程序)
#define so_loga(...) so_log_printf(SO_LT_ALWAYS, __VA_ARGS__)  // 总是输出

void so_log_printf(int t, const char *fmt, ...)
    __attribute__((format(printf, 2, 3)));

#ifdef __cplusplus
}
#endif

#endif
