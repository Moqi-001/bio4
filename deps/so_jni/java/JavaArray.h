#ifndef SO_JAVA_ARRAY_H
#define SO_JAVA_ARRAY_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../jni.h"
#include "JavaInstance.h"

enum {
    JAVA_ARRAY_TYPE_UNKNOWN = 0,
    JAVA_ARRAY_TYPE_BOOLEAN = 1,  // Z
    JAVA_ARRAY_TYPE_BYTE    = 2,  // B
    JAVA_ARRAY_TYPE_CHAR    = 3,  // C
    JAVA_ARRAY_TYPE_SHORT   = 4,  // S
    JAVA_ARRAY_TYPE_INT     = 5,  // I
    JAVA_ARRAY_TYPE_LONG    = 6,  // J
    JAVA_ARRAY_TYPE_FLOAT   = 7,  // F
    JAVA_ARRAY_TYPE_DOUBLE  = 8,  // D
    JAVA_ARRAY_TYPE_OBJECT  = 9,  // L
};

typedef void *JavaArray;
typedef void (*JavaElemsDtor_t)(void *elems, jsize length);

JavaArray JavaArrayNew(jsize length, jint type, JavaElemsDtor_t elems_dtor);

void   *JavaArrayRetainElements(JavaArray array);
int32_t JavaArrayReleaseElements(JavaArray array, void *elems);
void   *JavaArrayEditElements(JavaArray array);

jint  JavaArrayGetType(JavaArray array);
jsize JavaArrayGetLength(JavaArray array);
void *JavaArrayGetElements(JavaArray array);
jint  JavaArrayGetRegion(JavaArray array, jsize start, jsize length, void *dst);
jint  JavaArraySetRegion(JavaArray array, jsize start, jsize length, const void *src);

#ifdef __cplusplus
}
#endif

#endif