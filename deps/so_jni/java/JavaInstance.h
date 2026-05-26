#ifndef SO_JAVA_INSTANCE_H
#define SO_JAVA_INSTANCE_H

#include "../jni.h"

typedef void *JavaInstance;

typedef void (*JavaDtor_t)(void *data);

JavaInstance JavaInstanceNew(jsize size, JavaDtor_t dtor);
jint         JavaInstanceRetain(JavaInstance instance);
int32_t      JavaInstanceRelease(JavaInstance instance);
void         JavaInstanceReleaseForce(JavaInstance instance);

#endif