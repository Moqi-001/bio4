#include <stdlib.h>

#include "java/JavaBuffer.h"
#include "so_jni_internal.h"

#define JAVA_BUFFER_FROM_OBJ(_obj) ((JavaBuffer)JniGetObjectInstance(_obj))

jobject JniNewDirectByteBuffer(void *address, jlong capacity) {
    JavaBuffer buf = JavaBufferNewDirect(address, capacity);
    if (!buf) {
        return NULL;
    }

    jobject buffer = JniCreateObject(NULL, buf);
    if (!buffer) {
        JavaInstanceRelease(buf);
        return NULL;
    }

    return buffer;
}

void *JniGetDirectBufferAddress(jobject buffer) {
    jobject buf = JAVA_BUFFER_FROM_OBJ(buffer);
    if (!buf) return NULL;

    return JavaBufferGetData(buf);
}

jlong JniGetDirectBufferCapacity(jobject buffer) {
    jobject buf = JAVA_BUFFER_FROM_OBJ(buffer);
    if (!buf) return 0;

    return JavaBufferGetSize(buf);
}
