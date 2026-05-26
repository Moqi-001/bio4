#include <stddef.h>

#include "java/JavaString.h"
#include "so_jni_internal.h"

#define JAVA_STRING_FROM_OBJ(_obj) ((JavaString)JniGetObjectInstance(_obj))

jstring JniNewString(const jchar *utf16, jsize len) {
    JavaString str = JavaStringNewUtf16(utf16, len);
    if (!str) {
        return NULL;
    }

    jstring string = JniCreateObject(NULL, str);
    if (!string) {
        JavaInstanceRelease(str);
        return NULL;
    }

    JniAddLocalRef(string);

    return string;
}

jsize JniGetStringLength(jstring string) {
    JavaString str = JAVA_STRING_FROM_OBJ(string);
    if (!str) return 0;

    return JavaStringGetUtf16Length(str);
}

const jchar *JniGetStringChars(jstring string, jboolean *isCopy) {
    JavaString str = JAVA_STRING_FROM_OBJ(string);
    if (!str) return NULL;

    const jchar *utf16 = JavaStringRetainUtf16Chars(str);
    if (isCopy) *isCopy = JNI_FALSE;

    return utf16;
}

int32_t JniReleaseStringChars(jstring string, const jchar *utf16) {
    JavaString str = JAVA_STRING_FROM_OBJ(string);
    if (!str) return -1;

    return JavaStringReleaseUtf16Chars(str, utf16);
}

jint JniGetStringRegion(jstring string, jsize start, jsize length, jchar *buf) {
    JavaString str = JAVA_STRING_FROM_OBJ(string);
    if (!str) return JNI_EINVAL;

    return JavaStringGetUtf16Region(str, start, length, buf);
}

jstring JniNewStringUTF(const char *utf8) {
    JavaString str = JavaStringNewUtf8(utf8);
    if (!str) {
        return NULL;
    }

    jstring string = JniCreateObject(NULL, str);
    if (!string) {
        JavaInstanceRelease(str);
        return NULL;
    }

    JniAddLocalRef(string);

    return string;
}

jsize JniGetStringUTFLength(jstring string) {
    JavaString str = JAVA_STRING_FROM_OBJ(string);
    if (!str) return 0;

    return JavaStringGetUtf8Length(str);
}

const char *JniGetStringUTFChars(jstring string, jboolean *isCopy) {
    JavaString str = JAVA_STRING_FROM_OBJ(string);
    if (!str) return NULL;

    const char *utf8 = JavaStringRetainUtf8Chars(str);
    if (isCopy) *isCopy = JNI_FALSE;

    return utf8;
}

int32_t JniReleaseStringUTFChars(jstring string, const char *utf8) {
    JavaString str = JAVA_STRING_FROM_OBJ(string);
    if (!str) return -1;

    return JavaStringReleaseUtf8Chars(str, utf8);
}

jint JniGetStringUTFRegion(jstring string, jsize start, jsize length, char *buf) {
    JavaString str = JAVA_STRING_FROM_OBJ(string);
    if (!str) return JNI_EINVAL;

    return JavaStringGetUtf8Region(str, start, length, buf);
}
