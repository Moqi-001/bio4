#ifndef SO_JNI_INTERNAL_H
#define SO_JNI_INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "so_jni.h"

#include "so_jarray.h"
#include "so_jbuffer.h"
#include "so_jclass.h"
#include "so_jinstance.h"
#include "so_jobject.h"
#include "so_jref.h"
#include "so_jstring.h"

typedef struct so_jclass_node {
    jclass                *clazz;
    struct so_jclass_node *prev;
    struct so_jclass_node *next;
} so_jclass_node;

#ifdef __cplusplus
}
#endif

#endif