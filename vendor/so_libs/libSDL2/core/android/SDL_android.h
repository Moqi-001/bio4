#ifndef SDL_ANDROID_H
#define SDL_ANDROID_H

#include <so_jni/jni.h>

#ifdef __cplusplus
extern "C" {
#endif

#define __ANDROID__

#include <SDL2/SDL.h>

JNIEnv *Android_JNI_GetEnv(void);

int    Android_JNI_FileOpen(SDL_RWops  *ctx,
                            const char *fileName, const char *mode);
size_t Android_JNI_FileRead(SDL_RWops *ctx, void *buffer,
                            size_t size, size_t maxnum);
size_t Android_JNI_FileWrite(SDL_RWops *ctx, const void *buffer,
                             size_t size, size_t num);
Sint64 Android_JNI_FileSize(SDL_RWops *ctx);
Sint64 Android_JNI_FileSeek(SDL_RWops *ctx, Sint64 offset, int whence);
int    Android_JNI_FileClose(SDL_RWops *ctx);

void SDL_Android_Init(JNIEnv *mEnv, jclass cls);

#ifdef __cplusplus
}
#endif

#endif