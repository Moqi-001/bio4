#include "so_jni_internal.h"

jobject JniNewDirectByteBuffer(void *address, jlong capacity) {
    JBuffer buf = Buffer_newDirect(address, capacity, JNI_FALSE, JNI_FALSE);
    if (!buf) {
        return NULL;
    }

    jobject result = JniNewLocalRefFromJava(buf);
    Object_release(buf);

    return result;
}

void *JniGetDirectBufferAddress(jobject buffer) {
    return Buffer_getData(JniObjectToJava(buffer));
}

jlong JniGetDirectBufferCapacity(jobject buffer) {
    return Buffer_getSize(JniObjectToJava(buffer));
}
