#include <string.h>

#include <so_jni/so_jni.h>

// "com/eruption/util/VideoAdapter"

// static, 'start','(Ljava/lang/String;F)Z'
jboolean start(jargs *args) {  // String pFileName, float nVolume)
    const char *pFileName = JniGetStringUTFChars(JARG(args, jstring), NULL);
    jfloat      nVolume   = JARG(args, jfloat);
}

// static, "nowPlaying", "()Z"
jboolean nowPlaying() {
}