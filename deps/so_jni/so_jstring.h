#ifndef SO_JNI_JSTRING_H
#define SO_JNI_JSTRING_H

#ifdef __cplusplus
extern "C" {
#endif

#include "so_jni.h"

typedef struct so_jstring so_jstring;

so_jstring *so_jstring_new_with_utf16(const jchar *utf16, jsize len);
so_jstring *so_jstring_new_with_utf8(const char *utf8);

const jchar *so_jstring_retain_utf16_chars(so_jstring *string);
int32_t      so_jstring_release_utf16_chars(so_jstring *string, jchar *utf16);
jint         so_jstring_read_utf16_chars(so_jstring *string, jsize start, jsize length, jchar *dst);
const jchar *so_jstring_get_utf16_chars(so_jstring *string);
jsize        so_jstring_get_utf16_length(so_jstring *string);

const char *so_jstring_retain_utf8_chars(so_jstring *string);
int32_t     so_jstring_release_utf8_chars(so_jstring *string, char *utf8);
jint        so_jstring_read_utf8_chars(so_jstring *string, jsize start, jsize length, char *dst);
const char *so_jstring_get_utf8_chars(so_jstring *string);
jsize       so_jstring_get_utf8_length(so_jstring *string);

so_jstring *so_jstring_from_object(jstring string);

#ifdef __cplusplus
}
#endif

#endif