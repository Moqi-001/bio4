#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <psp2/io/fcntl.h>
#include <so_common/so_logger.h>
#include <so_font/so_font.h>
#include <so_jni/so_jni.h>

#include "config.h"
#include "java.h"

typedef struct TextBitmap {
    uint32_t reserved;
} TextBitmap;

static jint       m_nSizeByte   = 0;
static jint       m_nWidText    = 0;
static jint       m_nHeiText    = 0;
static jint       m_nWidStrText = 0;
static jint       m_nHeiStrText = 0;
static jint       m_FontSize    = 12;
static jbyteArray m_TextParam   = NULL;

static so_font *m_pFont = NULL;

static int FixTextTextureSize(int n) {  // 纹理大小需要是2的幂，要向上取
    // 文字纹理最小 16（对应字体最小 12 的渲染需求）
    if (n <= 16) return 16;
    int p = 32;
    while (p < n) p <<= 1;
    return p;
}

// static, "CreateTextBitmap", "(Ljava/lang/String;II)[B"
static jbyteArray CreateTextBitmap(jargs *args) {  // (String text, int x, int y)
    const char *text = JniGetStringUTFChars(JARG(args, jstring), NULL);
    jint        x    = JARG(args, jint);
    jint        y    = JARG(args, jint);

    m_nWidStrText = 0;
    m_nHeiStrText = 0;
    jbyte *bitmap = (jbyte *)so_font_create_bitmap(m_pFont, text, m_FontSize, 0, &m_nWidStrText, &m_nHeiStrText);

    m_nWidText  = FixTextTextureSize(m_nWidStrText);  // 文字纹理宽度
    m_nHeiText  = FixTextTextureSize(m_nHeiStrText);  // 文字纹理高度
    m_nSizeByte = m_nWidText * m_nHeiText;

    jbyteArray result      = JniNewByteArray(m_nSizeByte);
    jbyte     *resultBytes = JniGetByteArrayElements(result, NULL);
    memset(resultBytes, 0, m_nSizeByte);
    if (bitmap) {
        jbyte *src = bitmap;
        jbyte *dst = resultBytes;

        for (int y = 0; y < m_nHeiStrText; y++) {
            memcpy(dst, src, m_nWidStrText);
            src += m_nWidStrText;
            dst += m_nWidText;
        }
        free(bitmap);
    }
    JniReleaseByteArrayElements(result, resultBytes, 0);

    return result;
}

// static, "GetParam", "()[B"
static jbyteArray GetParam() {
    if (!m_TextParam) {
        // 创建 m_TextParam 全局引用
        m_TextParam = JniNewGlobalRefTake(JniNewByteArray(20));
        if (!m_TextParam) {
            so_loge("[TextBitmap] GetParam(): NewByteArray error! ");
            return NULL;
        }
    }

    jbyte *paramBytes = JniGetByteArrayElements(m_TextParam, NULL);
    // int 转小端
    paramBytes[0]  = (jbyte)(m_nSizeByte >> 0);
    paramBytes[1]  = (jbyte)(m_nSizeByte >> 8);
    paramBytes[2]  = (jbyte)(m_nSizeByte >> 16);
    paramBytes[3]  = (jbyte)(m_nSizeByte >> 24);
    paramBytes[4]  = (jbyte)(m_nWidText >> 0);
    paramBytes[5]  = (jbyte)(m_nWidText >> 8);
    paramBytes[6]  = (jbyte)(m_nWidText >> 16);
    paramBytes[7]  = (jbyte)(m_nWidText >> 24);
    paramBytes[8]  = (jbyte)(m_nHeiText >> 0);
    paramBytes[9]  = (jbyte)(m_nHeiText >> 8);
    paramBytes[10] = (jbyte)(m_nHeiText >> 16);
    paramBytes[11] = (jbyte)(m_nHeiText >> 24);
    paramBytes[12] = (jbyte)(m_nWidStrText >> 0);
    paramBytes[13] = (jbyte)(m_nWidStrText >> 8);
    paramBytes[14] = (jbyte)(m_nWidStrText >> 16);
    paramBytes[15] = (jbyte)(m_nWidStrText >> 24);
    paramBytes[16] = (jbyte)(m_nHeiStrText >> 0);
    paramBytes[17] = (jbyte)(m_nHeiStrText >> 8);
    paramBytes[18] = (jbyte)(m_nHeiStrText >> 16);
    paramBytes[19] = (jbyte)(m_nHeiStrText >> 24);

    JniReleaseByteArrayElements(m_TextParam, paramBytes, 0);

    // jobject 类型返回要增加局部引用给jni使用
    return JniNewLocalRef(m_TextParam);
}

// static, "SetupFontSize", "(I)I"
static jint SetupFontSize(jargs *args) {  // (int size)
    jint size = JARG(args, jint);

    m_FontSize = 12;
    for (int i = 50; i >= 12; i -= 2) {
        if (size >= so_font_get_font_height(m_pFont, i)) {
            m_FontSize = i;
            break;
        }
    }

    so_logd("[TextBitmap] SetupFontSize(%d): %d", size, m_FontSize);
    return m_FontSize;
}

static jsymbol class_symbols[] = {
    // 静态方法
    JNI_SYMBOL_STATIC_METHOD("CreateTextBitmap", "(Ljava/lang/String;II)[B", &CreateTextBitmap),
    JNI_SYMBOL_STATIC_METHOD("GetParam", "()[B", &GetParam),
    JNI_SYMBOL_STATIC_METHOD("SetupFontSize", "(I)I", &SetupFontSize),
};

static jclassMeta class_meta = {
    .name          = "jjp/co/capcom/android/googleplay/Evil4/TextBitmap",
    .super_name    = NULL,
    .symbols       = class_symbols,
    .num_symbols   = sizeof(class_symbols) / sizeof(class_symbols[0]),
    .instance_size = sizeof(TextBitmap),
};

void TextBitmap_class_init() {
    JniCreateClass(&class_meta);
    m_pFont = so_font_load(SO_TTF_PATH);
}
