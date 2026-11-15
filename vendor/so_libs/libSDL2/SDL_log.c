/*
 * SDL 日志函数：把 SDL_Log* 转成 so_log*，按优先级选日志级别。
 *
 * 不转发给真实 SDL2 的日志驱动——它往 stderr 写，在 PS Vita 上等于丢弃；而我们的日志
 * 系统有级别过滤、能落盘，也方便和引擎日志放在一起看。
 *
 * 级别对应关系：
 *   VERBOSE          -> so_logv（SO_LOG_LEVEL=1 才编进来）
 *   DEBUG            -> so_logd
 *   INFO             -> so_logi
 *   WARN             -> so_logw
 *   ERROR / CRITICAL -> so_loge（始终输出）
 * 每个分类的优先级由 SDL_LogSetPriority 控制，低于当前优先级的消息直接丢弃
 * （和 SDL2 自身行为一致，游戏用它静音某个分类时不会失效）。
 */
#include <stdarg.h>
#include <stdio.h>

#include <SDL2/SDL.h>

#include <so_common/so_logger.h>

#include "SDL_log_soloader.h"

#define SDL_LOG_CATEGORY_MAX 32
#define SDL_LOG_MESSAGE_MAX  1024   // SDL2 用 4096，这里按 Vita 的栈空间收小

// 每个分类当前的优先级；0 表示没设置过，按 SDL2 的默认值处理
static SDL_LogPriority s_log_priority[SDL_LOG_CATEGORY_MAX];

static SDL_LogPriority category_priority(int category) {
    if (category < 0 || category >= SDL_LOG_CATEGORY_MAX) {
        return SDL_LOG_PRIORITY_WARN;
    }

    if (s_log_priority[category]) {
        return s_log_priority[category];
    }

    // 与 SDL2 默认一致：APPLICATION 为 INFO，其余分类安静（WARN）
    return (category == SDL_LOG_CATEGORY_APPLICATION) ? SDL_LOG_PRIORITY_INFO
                                                      : SDL_LOG_PRIORITY_WARN;
}

static void log_printf(SDL_LogPriority priority, const char *fmt, va_list ap) {
    char buf[SDL_LOG_MESSAGE_MAX];
    vsnprintf(buf, sizeof(buf), fmt, ap);

    switch (priority) {
    case SDL_LOG_PRIORITY_VERBOSE: so_logv("[SDL] %s", buf); break;
    case SDL_LOG_PRIORITY_DEBUG:   so_logd("[SDL] %s", buf); break;
    case SDL_LOG_PRIORITY_INFO:   so_logi("[SDL] %s", buf); break;
    case SDL_LOG_PRIORITY_WARN:   so_logw("[SDL] %s", buf); break;
    default:                      so_loge("[SDL] %s", buf); break;   // ERROR / CRITICAL
    }
}

// SDL_Log 没有 category/priority 参数，等价于 SDL_LogMessage(APPLICATION, INFO)
void SDL_Log_soloader(const char *fmt, ...) {
    va_list ap;

    if (SDL_LOG_PRIORITY_INFO < category_priority(SDL_LOG_CATEGORY_APPLICATION)) {
        return;
    }

    va_start(ap, fmt);
    log_printf(SDL_LOG_PRIORITY_INFO, fmt, ap);
    va_end(ap);
}

#define SDL_LOG_IMPL(name, priority)                                 \
    void name##_soloader(int category, const char *fmt, ...) {       \
        va_list ap;                                                  \
                                                                     \
        if ((priority) < category_priority(category)) {              \
            return;                                                  \
        }                                                            \
                                                                     \
        va_start(ap, fmt);                                           \
        log_printf(priority, fmt, ap);                               \
        va_end(ap);                                                  \
    }

SDL_LOG_IMPL(SDL_LogVerbose,  SDL_LOG_PRIORITY_VERBOSE)
SDL_LOG_IMPL(SDL_LogDebug,    SDL_LOG_PRIORITY_DEBUG)
SDL_LOG_IMPL(SDL_LogInfo,     SDL_LOG_PRIORITY_INFO)
SDL_LOG_IMPL(SDL_LogWarn,     SDL_LOG_PRIORITY_WARN)
SDL_LOG_IMPL(SDL_LogError,    SDL_LOG_PRIORITY_ERROR)
SDL_LOG_IMPL(SDL_LogCritical, SDL_LOG_PRIORITY_CRITICAL)

void SDL_LogMessage_soloader(int category, SDL_LogPriority priority, const char *fmt, ...) {
    va_list ap;

    if (priority < category_priority(category)) {
        return;
    }

    va_start(ap, fmt);
    log_printf(priority, fmt, ap);
    va_end(ap);
}

void SDL_LogSetPriority_soloader(int category, SDL_LogPriority priority) {
    if (category >= 0 && category < SDL_LOG_CATEGORY_MAX) {
        s_log_priority[category] = priority;
    }
}

void SDL_LogSetAllPriority_soloader(SDL_LogPriority priority) {
    for (int i = 0; i < SDL_LOG_CATEGORY_MAX; i++) {
        s_log_priority[i] = priority;
    }
}

SDL_LogPriority SDL_LogGetPriority_soloader(int category) {
    return category_priority(category);
}
