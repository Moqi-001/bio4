#ifndef SO_JNI_JINSTANCE_H
#define SO_JNI_JINSTANCE_H

#include <so_jni/so_jni.h>

typedef void (*so_jdtor_t)(void *data);

void   *so_jinstance_new(jsize size, so_jdtor_t dtor);
jint    so_jinstance_retain(void *instance);
int32_t so_jinstance_release(void *instance);
void    so_jinstance_release_force(void *instance);

#endif