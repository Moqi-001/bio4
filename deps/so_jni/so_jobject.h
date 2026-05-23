#ifndef SO_JNI_JOBJECT_H
#define SO_JNI_JOBJECT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "so_jni.h"
#include "so_jref.h"

jobject so_jobject_new(jclass clazz, void *instance_size);
jobject so_jobject_retain(jobject object);
int32_t so_jobject_release(jobject object);
void    so_jobject_release_force(jobject object);
jobject so_jobject_edit(jobject object);
jobject so_jobject_attempt_edit(jobject object);

jclass         so_jobject_get_class(jobject object);
void          *so_jobject_get_instance(jobject object);
jobjectRefType so_jobject_get_ref_type(jobject object);
so_jref       *so_jobject_get_ref(jobject object);
jint           so_jobject_set_ref(jobject object, so_jref *ref);

#ifdef __cplusplus
}
#endif

#endif