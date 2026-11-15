#ifndef JAVA_NIO_BUFFER_H
#define JAVA_NIO_BUFFER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../lang/types.h"

typedef JObject JBuffer;

JBuffer Buffer_new(JLong size);
JBuffer Buffer_newDirect(void *address, JLong size, JBoolean copy, JBoolean adopt);

void *Buffer_getData(JBuffer buffer);
JLong Buffer_getSize(JBuffer buffer);

#ifdef __cplusplus
}
#endif

#endif