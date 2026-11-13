#ifndef ANDROID_STRING8_H
#define ANDROID_STRING8_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct AString8 AString8;

void AString8_ctor(AString8 *thiz);
void AString8_ctorWithCstr(AString8 *thiz, const char *cstr);
void AString8_ctorWithOther(AString8 *thiz, const AString8 *other);
void AString8_dtor(AString8 *thiz);

int AString8_setTo(AString8 *thiz, const AString8 *other);
int AString8_setToCstr(AString8 *thiz, const char *cstr);
int AString8_append(AString8 *thiz, const AString8 *other);
int AString8_appendCstr(AString8 *thiz, const char *cstr);
int AString8_appendPath(AString8 *thiz, const AString8 *other);
int AString8_appendPathCstr(AString8 *thiz, const char *cstr);
int AString8_getPathDir(AString8 *thiz, const AString8 *other);
int AString8_walkPath(AString8 *thiz, const AString8 *other, AString8 *out_remains);
int AString8_empty(AString8 *thiz);

const char *AString8_string(AString8 *thiz);
size_t      AString8_length(AString8 *thiz);
int         AString8_isEmpty(AString8 *thiz);

#ifdef __cplusplus
}
#endif

#endif