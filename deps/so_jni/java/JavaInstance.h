#ifndef SO_JAVA_INSTANCE_H
#define SO_JAVA_INSTANCE_H

#include "../jni.h"

typedef void (*JavaDtor_t)(void *data);

void   *JavaInstanceAlloc(jsize size, JavaDtor_t dtor);
jint    JavaInstanceRetain(void *instance);
int32_t JavaInstanceRelease(void *instance);
void    JavaInstanceReleaseForce(void *instance);

#endif