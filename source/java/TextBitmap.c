#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <psp2/io/fcntl.h>
#include <so_common/so_logger.h>
#include <so_jni/so_jni.h>

#include "android/FontMetrics.h"
#include "android/Paint.h"
#include "android/Typeface.h"

#include "config.h"
#include "java.h"

typedef struct TextBitmap {  // throws IOException
    uint32_t reserved;
} TextBitmap;

static jint       m_nSizeByte   = 0;
static jint       m_nWidText    = 0;
static jint       m_nHeiText    = 0;
static jint       m_nWidStrText = 0;
static jint       m_nHeiStrText = 0;
static jint       m_FontSize    = 12;
static jbyteArray m_TextParam   = NULL;

static jbyte m_Byte[] = {1, 2, 34, 4};

static int FixTextTextureSize(int n) {  // 纹理大小需要是2的幂次方，要向上取
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

    m_nSizeByte   = 1024;
    m_nWidStrText = m_FontSize;                         // 文字显示宽度
    m_nHeiStrText = m_FontSize;                         // 文字显示高度
    m_nWidText    = FixTextTextureSize(m_nWidStrText);  // 文字纹理宽度
    m_nHeiText    = FixTextTextureSize(m_nHeiStrText);  // 文字纹理高度

    jbyteArray bitmap = JniNewByteArray(m_nSizeByte);

    so_logd("[TextBitmap] CreateTextBitmap('%s',%d,%d)", text, x, y);
    return bitmap;
}

// static, "GetParam", "()[B"
static jbyteArray GetParam() {
    if (!m_TextParam) {
        // 创建 m_TextParam 全局引用
        m_TextParam = JniNewGlobalRefTake(JniNewByteArray(20));
        if (!m_TextParam) {
            so_loge("[TextBitmap] GetParam: NewByteArray error! ");
            return NULL;
        }
    }

    jbyte *bytes = JniGetByteArrayElements(m_TextParam, NULL);
    // int 转小端
    bytes[0]  = (jbyte)(m_nSizeByte >> 0);
    bytes[1]  = (jbyte)(m_nSizeByte >> 8);
    bytes[2]  = (jbyte)(m_nSizeByte >> 16);
    bytes[3]  = (jbyte)(m_nSizeByte >> 24);
    bytes[4]  = (jbyte)(m_nWidText >> 0);
    bytes[5]  = (jbyte)(m_nWidText >> 8);
    bytes[6]  = (jbyte)(m_nWidText >> 16);
    bytes[7]  = (jbyte)(m_nWidText >> 24);
    bytes[8]  = (jbyte)(m_nHeiText >> 0);
    bytes[9]  = (jbyte)(m_nHeiText >> 8);
    bytes[10] = (jbyte)(m_nHeiText >> 16);
    bytes[11] = (jbyte)(m_nHeiText >> 24);
    bytes[12] = (jbyte)(m_nWidStrText >> 0);
    bytes[13] = (jbyte)(m_nWidStrText >> 8);
    bytes[14] = (jbyte)(m_nWidStrText >> 16);
    bytes[15] = (jbyte)(m_nWidStrText >> 24);
    bytes[16] = (jbyte)(m_nHeiStrText >> 0);
    bytes[17] = (jbyte)(m_nHeiStrText >> 8);
    bytes[18] = (jbyte)(m_nHeiStrText >> 16);
    bytes[19] = (jbyte)(m_nHeiStrText >> 24);

    JniReleaseByteArrayElements(m_TextParam, bytes, 0);

    // jobject 类型返回要增加局部引用给jni使用
    return JniNewLocalRef(m_TextParam);
}

// static, "SetupFontSize", "(I)I"
static jint SetupFontSize(jargs *args) {  // (jint size)
    jint size = JARG(args, jint);

    m_FontSize = size;
    so_logd("[TextBitmap] SetupFontSize(%d)", size);
    return m_FontSize;

    APaint *paint = Paint_new();
    Paint_setAntiAlias(paint, true);
    Paint_setColor(paint, -1);
    Paint_setTypeface(paint, Typeface_DEFAULT_BOLD);

    m_FontSize = 12;

    jint i = 50;
    while (true) {
        if (i <= 12) {
            break;
        }
        Paint_setTextSize(paint, i);
        AFontMetrics *fontMetrics = Paint_getFontMetrics(paint);
        if (size < ((jint)(abs(fontMetrics->ascent) + fontMetrics->descent))) {
            i -= 2;
        } else {
            m_FontSize = i;
            break;
        }
    }

    so_logd("[TextBitmap] SetupFontSize(%d)", size);
    return m_FontSize;
}

static jsymbol class_symbols[] = {
    // 静态方法
    JNI_SYMBOL_STATIC_METHOD("CreateTextBitmap", "(Ljava/lang/String;II)[B", &CreateTextBitmap),
    JNI_SYMBOL_STATIC_METHOD("GetParam", "()[B", &GetParam),
    JNI_SYMBOL_STATIC_METHOD("SetupFontSize", "(I)I", &SetupFontSize),
};

static jclassMeta class_meta = {
    .name          = "jp/co/capcom/android/bio4_LGUplus0119/TextBitmap",
    .super_name    = NULL,
    .symbols       = class_symbols,
    .num_symbols   = sizeof(class_symbols) / sizeof(class_symbols[0]),
    .instance_size = sizeof(TextBitmap),
};

void TextBitmap_class_init() {
    JniCreateClass(&class_meta);
}
