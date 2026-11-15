/*
 * SDL 日志函数的移植层实现（SDL.c）声明。
 *
 * 本模块把 SDL_Log* 从「转发给真实 SDL2」改成输出到 so_log*，所以这些符号在
 * libSDL2_so 的符号表里指向 *_soloader，而不是真实 SDL2 的函数。
 */
#ifndef SO_LIBS_LIB_SDL2_SDL_LOG_SOLOADER_H
#define SO_LIBS_LIB_SDL2_SDL_LOG_SOLOADER_H

#include <SDL2/SDL.h>

void            SDL_Log_soloader(const char *fmt, ...);
void            SDL_LogVerbose_soloader(int category, const char *fmt, ...);
void            SDL_LogDebug_soloader(int category, const char *fmt, ...);
void            SDL_LogInfo_soloader(int category, const char *fmt, ...);
void            SDL_LogWarn_soloader(int category, const char *fmt, ...);
void            SDL_LogError_soloader(int category, const char *fmt, ...);
void            SDL_LogCritical_soloader(int category, const char *fmt, ...);
void            SDL_LogMessage_soloader(int category, SDL_LogPriority priority, const char *fmt, ...);
void            SDL_LogSetPriority_soloader(int category, SDL_LogPriority priority);
void            SDL_LogSetAllPriority_soloader(SDL_LogPriority priority);
SDL_LogPriority SDL_LogGetPriority_soloader(int category);

#endif
