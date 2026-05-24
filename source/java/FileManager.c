#include <psp2/io/fcntl.h>

#include "java.h"

typedef struct FileManager {
    uint32_t reserved;
} FileManager;

// static, "createFile", "(Ljava/lang/String;)Z"
static jboolean createFile(so_jargs *args) {  // (String pFileName)
    const char *pFileName = JniGetStringUTFChars(SO_JARG(args, jstring));

    SceUID fd = sceIoOpen(pFileName, SCE_O_WRONLY | SCE_O_CREAT, 0777);
    if (fd < 0) {
        return JNI_FALSE;
    }

    sceIoClose(fd);
    return JNI_TRUE;
}

// static, "deleteFile", "(Ljava/lang/String;)V"
static void deleteFile(so_jargs *args) {  // (String pFileName)
    const char *pFileName = JniGetStringUTFChars(SO_JARG(args, jstring));

    sceIoRemove(pFileName);
}

static so_jsymbol class_symbols[] = {
    // 静态方法
    SO_JSYMBOL_STATIC_METHOD("createFile", "(Ljava/lang/String;)Z", &createFile),
    SO_JSYMBOL_STATIC_METHOD("deleteFile", "(Ljava/lang/String;)V", &deleteFile),
};

static so_jclass_meta class_meta = {
    .name          = "jp/co/capcom/android/bio4_LGUplus0119/FileManager",
    .super_name    = NULL,
    .symbols       = class_symbols,
    .num_symbols   = sizeof(class_symbols) / sizeof(class_symbols[0]),
    .instance_size = sizeof(FileManager),
};

void FileManager_class_init() {
    so_jni_register_class_from_meta(&class_meta);
}
