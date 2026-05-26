#ifndef SO_JAVA_STRING_H
#define SO_JAVA_STRING_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../jni.h"
#include "JavaInstance.h"

typedef JavaInstance JavaString;

JavaString JavaStringNewUtf16(const jchar *utf16, jsize len);
JavaString JavaStringNewUtf8(const char *utf8);

const jchar *JavaStringRetainUtf16Chars(JavaString string);
int32_t      JavaStringReleaseUtf16Chars(JavaString string, const jchar *utf16);
jsize        JavaStringGetUtf16Length(JavaString string);
const jchar *JavaStringGetUtf16Chars(JavaString string);
jint         JavaStringGetUtf16Region(JavaString string, jsize start, jsize length, jchar *dst);

const char *JavaStringRetainUtf8Chars(JavaString string);
int32_t     JavaStringReleaseUtf8Chars(JavaString string, const char *utf8);
jsize       JavaStringGetUtf8Length(JavaString string);
const char *JavaStringGetUtf8Chars(JavaString string);
jint        JavaStringGetUtf8Region(JavaString string, jsize start, jsize length, char *dst);

#ifdef __cplusplus
}
#endif

#endif