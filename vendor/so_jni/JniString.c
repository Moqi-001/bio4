#include "so_jni_internal.h"

jstring JniNewString(const jchar *utf16, jsize len) {
    JString str = String_new(utf16, len);
    if (!str) {
        return NULL;
    }

    jstring result = JniNewLocalRefFromJava(str);
    Object_release(str);  // 由 LocalRef 托管

    return result;
}

jsize JniGetStringLength(jstring string) {
    return String_getLength(JniObjectToJava(string));
}

const jchar *JniGetStringChars(jstring string, jboolean *isCopy) {
    const jchar *result = String_getChars(JniObjectToJava(string));
    if (isCopy) {
        *isCopy = JNI_FALSE;
    }

    return result;
}

void JniReleaseStringChars(jstring string, const jchar *utf16) {
    // 无需做任何事
}

jint JniGetStringRegion(jstring string, jsize start, jsize length, jchar *buf) {
    return String_getRegion(JniObjectToJava(string), start, length, buf);
}

jstring JniNewStringUTF(const char *utf8) {
    JString str = String_newUTF(utf8);
    if (!str) {
        return NULL;
    }

    jstring result = JniNewLocalRefFromJava(str);
    Object_release(str);  // 由 LocalRef 托管

    return result;
}

jsize JniGetStringUTFLength(jstring string) {
    return String_getUTFLength(JniObjectToJava(string));
}

const char *JniGetStringUTFChars(jstring string, jboolean *isCopy) {
    const char *result = String_getUTFChars(JniObjectToJava(string));
    if (isCopy) {
        *isCopy = JNI_FALSE;
    }

    return result;
}

void JniReleaseStringUTFChars(jstring string, const char *utf8) {
    // 无需做任何事
}

jint JniGetStringUTFRegion(jstring string, jsize start, jsize length, char *buf) {
    return String_getUTFRegion(JniObjectToJava(string), start, length, buf);
}
