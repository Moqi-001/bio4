#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <so_common/so_logger.h>
#include <so_font/so_font.h>
#include <so_jni/so_jni.h>

#include "config.h"

static JClass sClass = NULL;

static JInt     m_nSizeByte   = 0;
static JInt     m_nWidText    = 0;
static JInt     m_nHeiText    = 0;
static JInt     m_nWidStrText = 0;
static JInt     m_nHeiStrText = 0;
static JInt     m_FontSize    = 12;
static JByte   *m_TextParam   = NULL;
static so_font *m_pFont       = NULL;

static int FixTextTextureSize(int n) {  // 纹理大小需要是2的幂，要向上取
    // 文字纹理最小 16（对应字体最小 12 的渲染需求）
    if (n <= 16) {
        return 16;
    }
    int p = 32;
    while (p < n) {
        p <<= 1;
    }
    return p;
}

static JByte *CreateTextBitmap(JString text, JInt x, JInt y) {
    const char *text_cstr = String_getUTFChars(text);
    so_bitmap  *bitmap    = so_font_create_text_bitmap(m_pFont, text_cstr, m_FontSize, 0);
    if (!bitmap) {
        m_nWidStrText = 0;
        m_nHeiStrText = 0;
        m_nWidText    = 0;
        m_nWidText    = 0;
        m_nSizeByte   = 0;
        so_loge("[TextBitmap] CreateTextBitmap(%s,%d,%d): error!", text_cstr, x, y);
        return Array_new(JTYPE_BYTE, 1);
    }

    m_nWidStrText = bitmap->width;
    m_nHeiStrText = bitmap->height;
    m_nWidText    = FixTextTextureSize(m_nWidStrText);  // 文字纹理宽度
    m_nHeiText    = FixTextTextureSize(m_nHeiStrText);  // 文字纹理高度
    m_nSizeByte   = m_nWidText * m_nHeiText;

    JByte *resultBytes = Array_new(JTYPE_BYTE, m_nSizeByte);
    memset(resultBytes, 0, m_nSizeByte);

    JByte *src = bitmap->pixels;
    JByte *dst = resultBytes;
    for (JInt y = 0; y < m_nHeiStrText; y++) {
        memcpy(dst, src, m_nWidStrText);
        src += bitmap->pitch;
        dst += m_nWidText;
    }
    free(bitmap);

    // so_logd("[TextBitmap] CreateTextBitmap(%s,%d,%d): %p", text_cstr, x, y, resultBytes);
    return resultBytes;
}

static JByte *GetParam() {
    // 返回后会被自动释放，m_TextParam下次还要用，需增加引用给jni
    JByte *paramBytes = Object_retain(m_TextParam);
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

    // so_logd("[TextBitmap] GetParam(): %p", paramBytes);
    return paramBytes;
}

static JInt SetupFontSize(JInt size) {
    m_FontSize = 12;
    for (JInt i = 50; i >= 12; i -= 2) {
        if (size >= so_font_get_font_height(m_pFont, i)) {
            m_FontSize = i;
            break;
        }
    }

    // so_logd("[TextBitmap] SetupFontSize(%d): %d", size, m_FontSize);
    return m_FontSize;
}

static JSymbol class_symbols[] = {
    // 静态方法
    JAVA_SYMBOL_STATIC_METHOD("CreateTextBitmap", "(Ljava/lang/String;II)[B", &CreateTextBitmap),
    JAVA_SYMBOL_STATIC_METHOD("GetParam", "()[B", &GetParam),
    JAVA_SYMBOL_STATIC_METHOD("SetupFontSize", "(I)I", &SetupFontSize),
};

static JClassMeta class_meta = {
    .name          = "jp/co/capcom/android/bio4_LGUplus0119/TextBitmap",
    .super_name    = NULL,
    .symbols       = class_symbols,
    .num_symbols   = sizeof(class_symbols) / sizeof(class_symbols[0]),
    .instance_size = 0,
};

void setup_TextBitmap() {
    sClass      = JavaCreateClass(&class_meta);
    m_pFont     = so_font_load(SO_TTF_PATH);
    m_TextParam = Array_new(JTYPE_BYTE, 20);
}
