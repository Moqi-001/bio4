#ifndef SO_JAVA_BUFFER_H
#define SO_JAVA_BUFFER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../jni.h"
#include "JavaInstance.h"

typedef void *JavaBuffer;

JavaBuffer JavaBufferNew(jlong size);
JavaBuffer JavaBufferNewDirect(void *address, jlong size);

void *JavaBufferGetData(JavaBuffer buffer);
jlong JavaBufferGetSize(JavaBuffer buffer);

#ifdef __cplusplus
}
#endif

#endif