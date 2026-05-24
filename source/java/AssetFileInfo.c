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
static bool calcInfo(so_jargs *args) {  // (String pFileName)
    const char *pFileName = JniGetStringUTFChars(SO_JARG(args, jstring));

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
    return JniCallNewStringUTF(m_pFilePath);
}

// static, "getStartOffset", "()J"
static jlong getStartOffset() {
    return m_nFileOffset;
}

// static, "getLength", "()J"
static jlong getLength() {
    return m_nFileLength;
}

static so_jsymbol class_symbols[] = {
    // 静态变量
    SO_JSYMBOL_STATIC_FIELD("m_nFileOffset", "J", &m_nFileOffset),
    SO_JSYMBOL_STATIC_FIELD("m_nFileLength", "J", &m_nFileLength),

    // 静态方法
    SO_JSYMBOL_STATIC_METHOD("calcInfo", "(Ljava/lang/String;)Z", &calcInfo),
    SO_JSYMBOL_STATIC_METHOD("getPackagePath", "()Ljava/lang/String;", &getPackagePath),
    SO_JSYMBOL_STATIC_METHOD("getStartOffset", "()J", &getStartOffset),
    SO_JSYMBOL_STATIC_METHOD("getLength", "()J", &getLength),
};

static so_jclass_meta class_meta = {
    .name          = "jp/co/capcom/android/bio4_LGUplus0119/AssetFileInfo",
    .super_name    = NULL,
    .symbols       = class_symbols,
    .num_symbols   = sizeof(class_symbols) / sizeof(class_symbols[0]),
    .instance_size = sizeof(AssetFileInfo),
};

void AssetFileInfo_class_init() {
    so_jni_register_class_from_meta(&class_meta);
}
