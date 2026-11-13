#ifndef JAVA_LANG_OBJECT_H
#define JAVA_LANG_OBJECT_H

#include "types.h"

JObject Object_new(JClass clazz);
JObject Object_newWithoutClass(JInt size, JDtor dtor);

JObject Object_retain(JObject object);
JInt    Object_release(JObject object);
JInt    Object_releaseKeep(JObject object);
void    Object_releaseForce(JObject object);
JInt    Object_retainWeak(JObject object, JObject *ref_ptr);
void    Object_releaseWeak(JObject object, JObject *ref_ptr);

JClass Object_getClass(JObject object);
JInt   Object_getSize(JObject object);

JBoolean Object_isInstanceOf(JObject obj, JClass clazz);

#endif