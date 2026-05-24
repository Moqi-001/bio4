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

// static, "CreateTextBitmap", "(Ljava/lang/String;II)[B"
static jbyteArray CreateTextBitmap(so_jargs *args) {  // (String text, int x, int y)
    const char *text = JniGetStringUTFChars(SO_JARG(args, jstring));
    jint        x    = SO_JARG(args, jint);
    jint        y    = SO_JARG(args, jint);

    m_nSizeByte   = 512;
    m_nWidText    = 12;
    m_nHeiText    = 12;
    m_nWidStrText = 12;
    m_nHeiStrText = 12;

    jbyteArray bitmap = JniCallNewByteArray(m_nSizeByte);

    so_logd("[TextBitmap] CreateTextBitmap(%s,%d,%d)", text, x, y);
    return bitmap;
}

// static, "GetParam", "()[B"
static jbyteArray GetParam() {
    if (!m_TextParam) {
        m_TextParam = JniNewArray(20, SO_JARRAY_TYPE_BYTE);
    }
    jbyte *bytes = JniGetArrayElems(m_TextParam);

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

    // jobject 类型返回要增加局部引用给jni使用
    return JniCallNewLocalRef(m_TextParam);
}

// static, "SetupFontSize", "(I)I"
static jint SetupFontSize(so_jargs *args) {  // (jint size)
    jint size = SO_JARG(args, jint);

    so_logd("[TextBitmap] SetupFontSize(%d)", size);

    m_FontSize = 12;
    return m_FontSize;

    APaint *paint = Paint_new();
    Paint_setAntiAlias(paint, true);
    Paint_setColor(paint, -1);
    Paint_setTypeface(paint, Typeface_DEFAULT_BOLD);

    int i = 50;
    while (true) {
        if (i <= 12) {
            break;
        }
        Paint_setTextSize(paint, i);
        AFontMetrics *fontMetrics = Paint_getFontMetrics(paint);
        if (size < ((int)(abs(fontMetrics->ascent) + fontMetrics->descent))) {
            i -= 2;
        } else {
            m_FontSize = i;
            break;
        }
    }
    return m_FontSize;
}

static so_jsymbol TextBitmap_symbols[] = {
    // 静态方法
    SO_JSYMBOL_STATIC_METHOD("CreateTextBitmap", "(Ljava/lang/String;II)[B", &CreateTextBitmap),
    SO_JSYMBOL_STATIC_METHOD("GetParam", "()[B", &GetParam),
    SO_JSYMBOL_STATIC_METHOD("SetupFontSize", "(I)I", &SetupFontSize),
};

so_jclass_meta TextBitmap_jclass_meta = {
    .name          = "jp/co/capcom/android/bio4_LGUplus0119/TextBitmap",
    .super_name    = NULL,
    .symbols       = TextBitmap_symbols,
    .num_symbols   = sizeof(TextBitmap_symbols) / sizeof(TextBitmap_symbols[0]),
    .instance_size = sizeof(TextBitmap),
};