#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <psp2/io/fcntl.h>
#include <so_common/so_logger.h>
#include <so_jni/so_jni.h>

#include "config.h"

// 每次读取文件都要读取 apk，用假的文件代替
#define PACKAGE_PATH "app0:eboot.bin"

typedef struct AssetFileInfo {  // throws IOException
    uint32_t reserved;
} AssetFileInfo;

static jlong m_nFileOffset;
static jlong m_nFileLength;

// static "calcInfo", "(Ljava/lang/String;)Z"
bool AssetFileInfo_calcInfo(so_jargs *args) {
    jstring pFileName = SO_JARG(args, jstring);

    char        path[1024];
    const char *fileName = so_jstring_get_utf8_chars(so_jobject_get_instance(pFileName));

    snprintf(path, sizeof(path), "%s%s.png", SO_ASSETS_DIR, fileName);

    SceUID fd = sceIoOpen(path, SCE_O_RDONLY, 0);
    if (fd < 0) {
        m_nFileOffset = 0;
        m_nFileLength = 0;
        so_loge("[AssetFileInfo] calcInfo('%s'): '%s' not exists!", fileName, path);
        return false;
    }

    m_nFileOffset = 0;
    m_nFileLength = sceIoLseek32(fd, 0, SCE_SEEK_END);

    sceIoClose(fd);

    so_logd("[AssetFileInfo] calcInfo('%s'): length=%lld", fileName, m_nFileLength);
    return true;
}

// static, "getPackagePath", "()Ljava/lang/String;"
jstring AssetFileInfo_getPackagePath() {
    // return DeviceInfo.getPackagePath();
    return (_jni_env)->NewStringUTF(&_jni_env, PACKAGE_PATH);
}

// static, "getStartOffset", "()J"
jlong AssetFileInfo_getStartOffset() {
    return m_nFileOffset;
}

// static, "getLength", "()J"
jlong AssetFileInfo_getLength() {
    return m_nFileLength;
}

static so_jsymbol AssetFileInfo_symbols[] = {
    // 静态变量
    SO_JSYMBOL_STATIC_FIELD("m_nFileOffset", "J", &m_nFileOffset),
    SO_JSYMBOL_STATIC_FIELD("m_nFileLength", "J", &m_nFileLength),

    // 静态方法
    SO_JSYMBOL_STATIC_METHOD("calcInfo", "(Ljava/lang/String;)Z", &AssetFileInfo_calcInfo),
    SO_JSYMBOL_STATIC_METHOD("getPackagePath", "()Ljava/lang/String;", &AssetFileInfo_getPackagePath),
    SO_JSYMBOL_STATIC_METHOD("getStartOffset", "()J", &AssetFileInfo_getStartOffset),
    SO_JSYMBOL_STATIC_METHOD("getLength", "()J", &AssetFileInfo_getLength),
};

so_jclass_meta AssetFileInfo_jclass_meta = {
    .name          = "jp/co/capcom/android/bio4_LGUplus0119/AssetFileInfo",
    .super_name    = NULL,
    .symbols       = AssetFileInfo_symbols,
    .num_symbols   = sizeof(AssetFileInfo_symbols) / sizeof(AssetFileInfo_symbols[0]),
    .instance_size = sizeof(AssetFileInfo),
};
