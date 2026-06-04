#include <psp2/io/fcntl.h>

#include "java.h"

typedef struct FileManager {
    uint32_t reserved;
} FileManager;

// static, "createFile", "(Ljava/lang/String;)Z"
static jboolean createFile(jargs *args) {  // (String pFileName)
    const char *pFileName = JniGetStringUTFChars(JARG(args, jstring), NULL);

    SceUID fd = sceIoOpen(pFileName, SCE_O_WRONLY | SCE_O_CREAT, 0777);
    if (fd < 0) {
        return JNI_FALSE;
    }

    sceIoClose(fd);
    return JNI_TRUE;
}

// static, "deleteFile", "(Ljava/lang/String;)V"
static void deleteFile(jargs *args) {  // (String pFileName)
    const char *pFileName = JniGetStringUTFChars(JARG(args, jstring), NULL);

    sceIoRemove(pFileName);
}

static jsymbol class_symbols[] = {
    // 静态方法
    JNI_SYMBOL_STATIC_METHOD("createFile", "(Ljava/lang/String;)Z", &createFile),
    JNI_SYMBOL_STATIC_METHOD("deleteFile", "(Ljava/lang/String;)V", &deleteFile),
};

static jclassMeta class_meta = {
    .name          = "jjp/co/capcom/android/googleplay/Evil4/FileManager",
    .super_name    = NULL,
    .symbols       = class_symbols,
    .num_symbols   = sizeof(class_symbols) / sizeof(class_symbols[0]),
    .instance_size = sizeof(FileManager),
};

void FileManager_class_init() {
    JniCreateClass(&class_meta);
}
