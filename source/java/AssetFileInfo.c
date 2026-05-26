#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <psp2/io/fcntl.h>
#include <so_common/so_logger.h>
#include <so_jni/so_jni.h>

#include "config.h"
#include "java.h"

typedef struct AssetFileInfo {
    uint32_t reserved;
} AssetFileInfo;

static jlong m_nFileOffset;
static jlong m_nFileLength;
static char  m_pFilePath[1024];

// static "calcInfo", "(Ljava/lang/String;)Z"
static bool calcInfo(jargs *args) {  // (String pFileName)
    const char *pFileName = JniGetStringUTFChars(JARG(args, jstring), NULL);

    snprintf(m_pFilePath, sizeof(m_pFilePath), "%s%s.png", SO_ASSETS_DIR, pFileName);

    SceUID fd = sceIoOpen(m_pFilePath, SCE_O_RDONLY, 0);
    if (fd < 0) {
        m_nFileOffset = 0;
        m_nFileLength = 0;
        so_loge("[AssetFileInfo] calcInfo('%s'): '%s' not exists!", pFileName, m_pFilePath);
        return false;
    }

    m_nFileOffset = 0;
    m_nFileLength = sceIoLseek32(fd, 0, SCE_SEEK_END);

    sceIoClose(fd);

    so_logd("[AssetFileInfo] calcInfo('%s'): length=%lld", pFileName, m_nFileLength);
    return true;
}

// static, "getPackagePath", "()Ljava/lang/String;"
static jstring getPackagePath() {
    return JniNewStringUTF(m_pFilePath);
}

// static, "getStartOffset", "()J"
static jlong getStartOffset() {
    return m_nFileOffset;
}

// static, "getLength", "()J"
static jlong getLength() {
    return m_nFileLength;
}

static jsymbol class_symbols[] = {
    // 静态变量
    JNI_SYMBOL_STATIC_FIELD("m_nFileOffset", "J", &m_nFileOffset),
    JNI_SYMBOL_STATIC_FIELD("m_nFileLength", "J", &m_nFileLength),

    // 静态方法
    JNI_SYMBOL_STATIC_METHOD("calcInfo", "(Ljava/lang/String;)Z", &calcInfo),
    JNI_SYMBOL_STATIC_METHOD("getPackagePath", "()Ljava/lang/String;", &getPackagePath),
    JNI_SYMBOL_STATIC_METHOD("getStartOffset", "()J", &getStartOffset),
    JNI_SYMBOL_STATIC_METHOD("getLength", "()J", &getLength),
};

static jclassMeta class_meta = {
    .name          = "jp/co/capcom/android/bio4_LGUplus0119/AssetFileInfo",
    .super_name    = NULL,
    .symbols       = class_symbols,
    .num_symbols   = sizeof(class_symbols) / sizeof(class_symbols[0]),
    .instance_size = sizeof(AssetFileInfo),
};

void AssetFileInfo_class_init() {
    JniCreateClass(&class_meta);
}
