#ifndef JAVA_LANG_SYSTEM_H
#define JAVA_LANG_SYSTEM_H

#include <so_jni/so_jni.h>

extern so_jclass_meta JSystem_jclass_meta;

jlong JSystem_currentTimeMillis();

#endif