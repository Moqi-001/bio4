#ifndef SO_JNI_JBUFFER_H
#define SO_JNI_JBUFFER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "so_jni.h"

typedef struct so_jbuffer so_jbuffer;

so_jbuffer *so_jbuffer_new(jlong size);
so_jbuffer *so_jbuffer_new_direct(void *address, jlong size);

void *so_jbuffer_get_data(so_jbuffer *buffer);
jlong so_jbuffer_get_size(so_jbuffer *buffer);

so_jbuffer *so_jbuffer_from_object(jobject obj);

#ifdef __cplusplus
}
#endif

#endif