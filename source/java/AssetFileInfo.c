#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <psp2/io/fcntl.h>
#include <so_common/so_logger.h>
#include <so_jni/so_jni.h>

#include "config.h"

struct _AssetFileInfo {
    uint32_t reserved;
};
typedef struct _AssetFileInfo *AssetFileInfo;

static JClass sClass = NULL;

static JLong m_nFileOffset;
static JLong m_nFileLength;
static char  m_pFilePath[1024];

static JBoolean calcInfo(JString pFileName) {
    const char *pFileName_cstr = String_getUTFChars(pFileName);

    snprintf(m_pFilePath, sizeof(m_pFilePath), "%s%s.png", SO_ASSETS_DIR, pFileName_cstr);

    SceUID fd = sceIoOpen(m_pFilePath, SCE_O_RDONLY, 0);
    if (fd < 0) {
        m_nFileOffset = 0;
        m_nFileLength = 0;
        so_loge("[AssetFileInfo] calcInfo('%s'): '%s' not exists!", pFileName_cstr, m_pFilePath);
        return false;
    }

    m_nFileOffset = 0;
    m_nFileLength = sceIoLseek32(fd, 0, SCE_SEEK_END);

    sceIoClose(fd);

    so_logd("[AssetFileInfo] calcInfo('%s'): length=%lld", pFileName_cstr, m_nFileLength);
    return true;
}

static JString getPackagePath() {
    return String_newUTF(m_pFilePath);
}

static JLong getStartOffset() {
    return m_nFileOffset;
}

static JLong getLength() {
    return m_nFileLength;
}

static JSymbol class_symbols[] = {
    // 静态变量
    JAVA_SYMBOL_STATIC_FIELD("m_nFileOffset", "J", &m_nFileOffset),
    JAVA_SYMBOL_STATIC_FIELD("m_nFileLength", "J", &m_nFileLength),

    // 静态方法
    JAVA_SYMBOL_STATIC_METHOD("calcInfo", "(Ljava/lang/String;)Z", &calcInfo),
    JAVA_SYMBOL_STATIC_METHOD("getPackagePath", "()Ljava/lang/String;", &getPackagePath),
    JAVA_SYMBOL_STATIC_METHOD("getStartOffset", "()J", &getStartOffset),
    JAVA_SYMBOL_STATIC_METHOD("getLength", "()J", &getLength),
};

static JClassMeta class_meta = {
    .name          = "jp/co/capcom/android/bio4_LGUplus0119/AssetFileInfo",
    .super_name    = NULL,
    .symbols       = class_symbols,
    .num_symbols   = sizeof(class_symbols) / sizeof(class_symbols[0]),
    .instance_size = sizeof(struct _AssetFileInfo),
};

void setup_AssetFileInfo() {
    sClass = JavaCreateClass(&class_meta);
}
