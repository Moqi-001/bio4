#ifndef SO_JNI_JARRAY_H
#define SO_JNI_JARRAY_H

#ifdef __cplusplus
extern "C" {
#endif

#include "so_jni.h"

enum {
    SO_JARRAY_TYPE_UNKNOWN = 0,
    SO_JARRAY_TYPE_BOOLEAN = 1,  // Z
    SO_JARRAY_TYPE_BYTE    = 2,  // B
    SO_JARRAY_TYPE_CHAR    = 3,  // C
    SO_JARRAY_TYPE_SHORT   = 4,  // S
    SO_JARRAY_TYPE_INT     = 5,  // I
    SO_JARRAY_TYPE_LONG    = 6,  // J
    SO_JARRAY_TYPE_FLOAT   = 7,  // F
    SO_JARRAY_TYPE_DOUBLE  = 8,  // D
    SO_JARRAY_TYPE_OBJECT  = 9,  // L
};

typedef struct so_jarray so_jarray;

so_jarray *so_jarray_new(jsize length, jint type);

void   *so_jarray_retain_elems(so_jarray *array);
int32_t so_jarray_release_elems(so_jarray *array, void *elems);
void   *so_jarray_edit_elems(so_jarray *array);
jint    so_jarray_read_elems(so_jarray *array, jsize start, jsize length, void *dst);
jint    so_jarray_write_elems(so_jarray *array, jsize start, jsize length, const void *src);

jint  so_jarray_get_type(so_jarray *array);
void *so_jarray_get_elems(so_jarray *array);
jsize so_jarray_get_length(so_jarray *array);

so_jarray *so_jarray_from_object(jobject obj);

#ifdef __cplusplus
}
#endif

#endif