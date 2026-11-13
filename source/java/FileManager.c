#include <psp2/io/fcntl.h>
#include <so_jni/so_jni.h>

struct _FileManager {
    uint32_t reserved;
};
typedef struct _FileManager *FileManager;

static JClass sClass = NULL;

static JBoolean createFile(JString pFileName) {
    const char *pFileName_cstr = String_getUTFChars(pFileName);

    SceUID fd = sceIoOpen(pFileName_cstr, SCE_O_WRONLY | SCE_O_CREAT, 0777);
    if (fd < 0) {
        return JNI_FALSE;
    }

    sceIoClose(fd);
    return JNI_TRUE;
}

static void deleteFile(JString pFileName) {
    const char *pFileName_cstr = String_getUTFChars(pFileName);

    sceIoRemove(pFileName_cstr);
}

static JSymbol class_symbols[] = {
    // 静态方法
    JAVA_SYMBOL_STATIC_METHOD("createFile", "(Ljava/lang/String;)Z", &createFile),
    JAVA_SYMBOL_STATIC_METHOD("deleteFile", "(Ljava/lang/String;)V", &deleteFile),
};

static JClassMeta class_meta = {
    .name          = "jp/co/capcom/android/bio4_LGUplus0119/FileManager",
    .super_name    = NULL,
    .symbols       = class_symbols,
    .num_symbols   = sizeof(class_symbols) / sizeof(class_symbols[0]),
    .instance_size = sizeof(struct _FileManager),
};

void setup_FileManager() {
    sClass = JavaCreateClass(&class_meta);
}
