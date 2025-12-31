#ifndef ANDROID_STRING8_H
#define ANDROID_STRING8_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct String8 String8;

int  android_String8_init(String8 *str8);
int  android_String8_initFromCstring(String8 *str8, const char *s);
int  android_String8_initFromString8(String8 *str8, const String8 *src);
void android_String8_term(String8 *str8);

int android_String8_setTo(String8 *str8, const String8 *src);
int android_String8_setToCstring(String8 *str8, const char *s);
int android_String8_append(String8 *str8, const String8 *src);
int android_String8_appendCstring(String8 *str8, const char *s);
int android_String8_appendPath(String8 *str8, const String8 *src);
int android_String8_appendPathCstring(String8 *str8, const char *s);
int android_String8_getPathDir(String8 *str8, const String8 *src);
int android_String8_walkPath(String8 *str8, const String8 *src, String8 *out_remains);
int android_String8_empty(String8 *str8);

const char *android_String8_string(String8 *str8);
size_t      android_String8_length(String8 *str8);
int         android_String8_isEmpty(String8 *str8);

#ifdef __cplusplus
}
#endif

#endif