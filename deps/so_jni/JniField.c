#include "so_jni_internal.h"

jfieldID JniGetFieldID(const jclass clazz, const char *name, const char *sig) {
    const JSymbol *symbol = Class_findFieldSymbol(JniObjectToJava(clazz), name, sig, JNI_FALSE);

    return (jfieldID)symbol;
}

jfieldID JniGetStaticFieldID(const jclass clazz, const char *name, const char *sig) {
    const JSymbol *symbol = Class_findFieldSymbol(JniObjectToJava(clazz), name, sig, JNI_TRUE);

    return (jfieldID)symbol;
}
