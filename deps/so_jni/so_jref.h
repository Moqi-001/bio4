#ifndef SO_JNI_JFEF_H
#define SO_JNI_JFEF_H

#ifdef __cplusplus
extern "C" {
#endif

#include "so_jni.h"

typedef struct so_jframe so_jframe;
typedef struct so_jref   so_jref;

jobjectRefType so_jref_get_type(so_jref *ref);

jint    so_jref_take_local(jobject object);
jobject so_jref_new_local(jobject object);
void    so_jref_delete_local(jobject object);
jobject so_jref_new_global(jobject object);
void    so_jref_delete_global(jobject object);

jint    so_jref_push_local_frame(jint capacity);
jobject so_jref_pop_local_frame(jobject result);

jint so_jref_global_init(void);
void so_jref_global_fini(void);

#ifdef __cplusplus
}
#endif

#endif