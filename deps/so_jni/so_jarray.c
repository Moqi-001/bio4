#include <stddef.h>

#include "java/JavaArray.h"
#include "so_jni_internal.h"

#define JAVA_ARRAY_FROM_OBJ(_obj) ((JavaArray)JniGetObjectInstance(_obj))

static jarray *_jarray_new(jsize length, jint type) {
    JavaArray arr = JavaArrayNew(length, type, NULL);
    if (!arr) {
        return NULL;
    }

    jarray array = JniCreateObject(NULL, arr);
    if (!array) {
        JavaInstanceRelease(arr);
        return NULL;
    }

    return array;
}

static void *_jarray_get_elements(jarray array, jboolean *isCopy) {
    JavaArray arr = JAVA_ARRAY_FROM_OBJ(array);
    if (!arr) {
        return NULL;
    }

    void *ori_elems = JavaArrayGetElements(arr);
    void *elems     = JavaArrayRetainElements(arr);
    if (!elems) {
        return NULL;
    }

    if (isCopy) *isCopy = (ori_elems != elems);

    return elems;
}

static void _jarray_release_elements(jarray array, void *elems, jint mode) {
    JavaArray arr = JAVA_ARRAY_FROM_OBJ(array);

    if (mode != JNI_ABORT) {
        JavaArraySetRegion(arr, 0, JavaArrayGetLength(arr), elems);
    }

    if (mode != JNI_COMMIT) {
        JavaArrayReleaseElements(arr, elems);
    }
}

static void _jarray_get_region(jarray array, jsize start, jsize len, void *buf) {
    JavaArray arr = JAVA_ARRAY_FROM_OBJ(array);
    JavaArrayGetRegion(arr, start, len, buf);
}

static void _jarray_set_region(jarray array, jsize start, jsize len, const void *buf) {
    JavaArray arr = JAVA_ARRAY_FROM_OBJ(array);
    JavaArraySetRegion(arr, start, len, buf);
}

static void _jobject_elems_dtor(jobject *elems, jsize length) {
    if (!elems || length <= 0) return;

    for (jint i = 0; i < length; i++) {
        if (elems[i]) {
            JniDeleteLocalRef(elems[i]);
        }
    }
}

jlong JniGetArrayLength(jarray array) {
    JavaArray arr = JAVA_ARRAY_FROM_OBJ(array);
    if (!arr) {
        return 0;
    }

    return JavaArrayGetLength(arr);
}

jobjectArray JniNewObjectArray(jsize length, jclass elementClass, jobject initialElement) {
    if (length <= 0) {
        return NULL;
    }

    if (initialElement && elementClass) {
        jclass initialClass = JniGetObjectClass(initialElement);
        if (!initialClass) {
            return NULL;
        }
        if (!JniIsAssignableFrom(initialClass, elementClass)) {
            return NULL;
        }
    }

    JavaArray arr = JavaArrayNew(length, JAVA_ARRAY_TYPE_OBJECT, (JavaElemsDtor_t)_jobject_elems_dtor);
    if (!arr) {
        return NULL;
    }

    jarray array = JniCreateObject(NULL, arr);
    if (!array) {
        JavaInstanceRelease(arr);
        return NULL;
    }

    jobject *elems = (jobject *)JavaArrayGetElements(arr);
    if (elems) {
        for (jsize i = 0; i < length; i++) {
            elems[i] = JniNewLocalRef(initialElement);
        }
    }

    return array;
}

jobject JniGetObjectArrayElement(jobjectArray array, jsize index) {
    JavaArray arr = JAVA_ARRAY_FROM_OBJ(array);
    if (!arr) {
        return NULL;
    }

    jobject *elems  = (jobject *)JavaArrayGetElements(arr);
    jsize    length = JavaArrayGetLength(arr);
    if (!elems || index < 0 || index >= length) {
        return NULL;
    }

    return JniNewLocalRef(elems[index]);
}

void JniSetObjectArrayElement(jobjectArray array, jsize index, jobject value) {
    JavaArray arr = JAVA_ARRAY_FROM_OBJ(array);
    if (!arr) {
        return;
    }

    jobject *elems  = (jobject *)JavaArrayGetElements(arr);
    jsize    length = JavaArrayGetLength(arr);
    if (!elems || index < 0 || index >= length) {
        return;
    }

    if (elems[index]) {
        JniDeleteLocalRef(elems[index]);
    }

    elems[index] = JniNewLocalRef(value);
}

jbooleanArray JniNewBooleanArray(jsize length) {
    return (jbooleanArray)_jarray_new(length, JAVA_ARRAY_TYPE_BOOLEAN);
}
jbyteArray JniNewByteArray(jsize length) {
    return (jbyteArray)_jarray_new(length, JAVA_ARRAY_TYPE_BYTE);
}
jcharArray JniNewCharArray(jsize length) {
    return (jcharArray)_jarray_new(length, JAVA_ARRAY_TYPE_CHAR);
}
jshortArray JniNewShortArray(jsize length) {
    return (jshortArray)_jarray_new(length, JAVA_ARRAY_TYPE_SHORT);
}
jintArray JniNewIntArray(jsize length) {
    return (jintArray)_jarray_new(length, JAVA_ARRAY_TYPE_INT);
}
jlongArray JniNewLongArray(jsize length) {
    return (jlongArray)_jarray_new(length, JAVA_ARRAY_TYPE_LONG);
}
jfloatArray JniNewFloatArray(jsize length) {
    return (jfloatArray)_jarray_new(length, JAVA_ARRAY_TYPE_FLOAT);
}
jdoubleArray JniNewDoubleArray(jsize length) {
    return (jdoubleArray)_jarray_new(length, JAVA_ARRAY_TYPE_DOUBLE);
}

jboolean *JniGetBooleanArrayElements(jbooleanArray array, jboolean *isCopy) {
    return (jboolean *)_jarray_get_elements(array, isCopy);
}
jbyte *JniGetByteArrayElements(jbyteArray array, jboolean *isCopy) {
    return (jbyte *)_jarray_get_elements(array, isCopy);
}
jchar *JniGetCharArrayElements(jcharArray array, jboolean *isCopy) {
    return (jchar *)_jarray_get_elements(array, isCopy);
}
jshort *JniGetShortArrayElements(jshortArray array, jboolean *isCopy) {
    return (jshort *)_jarray_get_elements(array, isCopy);
}
jint *JniGetIntArrayElements(jintArray array, jboolean *isCopy) {
    return (jint *)_jarray_get_elements(array, isCopy);
}
jlong *JniGetLongArrayElements(jlongArray array, jboolean *isCopy) {
    return (jlong *)_jarray_get_elements(array, isCopy);
}
jfloat *JniGetFloatArrayElements(jfloatArray array, jboolean *isCopy) {
    return (jfloat *)_jarray_get_elements(array, isCopy);
}
jdouble *JniGetDoubleArrayElements(jdoubleArray array, jboolean *isCopy) {
    return (jdouble *)_jarray_get_elements(array, isCopy);
}
void JniReleaseBooleanArrayElements(jbooleanArray array, jboolean *elems, jint mode) {
    _jarray_release_elements(array, (void *)elems, mode);
}
void JniReleaseByteArrayElements(jbyteArray array, jbyte *elems, jint mode) {
    _jarray_release_elements(array, (void *)elems, mode);
}
void JniReleaseCharArrayElements(jcharArray array, jchar *elems, jint mode) {
    _jarray_release_elements(array, (void *)elems, mode);
}
void JniReleaseShortArrayElements(jshortArray array, jshort *elems, jint mode) {
    _jarray_release_elements(array, (void *)elems, mode);
}
void JniReleaseIntArrayElements(jintArray array, jint *elems, jint mode) {
    _jarray_release_elements(array, (void *)elems, mode);
}
void JniReleaseLongArrayElements(jlongArray array, jlong *elems, jint mode) {
    _jarray_release_elements(array, (void *)elems, mode);
}
void JniReleaseFloatArrayElements(jfloatArray array, jfloat *elems, jint mode) {
    _jarray_release_elements(array, (void *)elems, mode);
}
void JniReleaseDoubleArrayElements(jdoubleArray array, jdouble *elems, jint mode) {
    _jarray_release_elements(array, (void *)elems, mode);
}

void JniGetBooleanArrayRegion(jbooleanArray array, jsize start, jsize len, jboolean *buf) {
    _jarray_get_region(array, start, len, (void *)buf);
}
void JniGetByteArrayRegion(jbyteArray array, jsize start, jsize len, jbyte *buf) {
    _jarray_get_region(array, start, len, (void *)buf);
}
void JniGetCharArrayRegion(jcharArray array, jsize start, jsize len, jchar *buf) {
    _jarray_get_region(array, start, len, (void *)buf);
}
void JniGetShortArrayRegion(jshortArray array, jsize start, jsize len, jshort *buf) {
    _jarray_get_region(array, start, len, (void *)buf);
}
void JniGetIntArrayRegion(jintArray array, jsize start, jsize len, jint *buf) {
    _jarray_get_region(array, start, len, (void *)buf);
}
void JniGetLongArrayRegion(jlongArray array, jsize start, jsize len, jlong *buf) {
    _jarray_get_region(array, start, len, (void *)buf);
}
void JniGetFloatArrayRegion(jfloatArray array, jsize start, jsize len, jfloat *buf) {
    _jarray_get_region(array, start, len, (void *)buf);
}
void JniGetDoubleArrayRegion(jdoubleArray array, jsize start, jsize len, jdouble *buf) {
    _jarray_get_region(array, start, len, (void *)buf);
}
void JniSetBooleanArrayRegion(jbooleanArray array, jsize start, jsize len, const jboolean *buf) {
    _jarray_set_region(array, start, len, (void *)buf);
}
void JniSetByteArrayRegion(jbyteArray array, jsize start, jsize len, const jbyte *buf) {
    _jarray_set_region(array, start, len, (void *)buf);
}
void JniSetCharArrayRegion(jcharArray array, jsize start, jsize len, const jchar *buf) {
    _jarray_set_region(array, start, len, (void *)buf);
}
void JniSetShortArrayRegion(jshortArray array, jsize start, jsize len, const jshort *buf) {
    _jarray_set_region(array, start, len, (void *)buf);
}
void JniSetIntArrayRegion(jintArray array, jsize start, jsize len, const jint *buf) {
    _jarray_set_region(array, start, len, (void *)buf);
}
void JniSetLongArrayRegion(jlongArray array, jsize start, jsize len, const jlong *buf) {
    _jarray_set_region(array, start, len, (void *)buf);
}
void JniSetFloatArrayRegion(jfloatArray array, jsize start, jsize len, const jfloat *buf) {
    _jarray_set_region(array, start, len, (void *)buf);
}
void JniSetDoubleArrayRegion(jdoubleArray array, jsize start, jsize len, const jdouble *buf) {
    _jarray_set_region(array, start, len, (void *)buf);
}

void *JniGetPrimitiveArrayCritical(jarray array, jboolean *isCopy) {
    return _jarray_get_elements(array, isCopy);
}
void JniReleasePrimitiveArrayCritical(jarray array, void *carray, jint mode) {
    _jarray_release_elements(array, carray, mode);
}
