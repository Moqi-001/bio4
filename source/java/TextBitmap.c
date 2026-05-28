#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <psp2/io/fcntl.h>
#include <so_common/so_logger.h>
#include <so_jni/so_jni.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "android/FontMetrics.h"
#include "android/Paint.h"
#include "android/Typeface.h"

#include "config.h"
#include "java.h"

typedef struct TextBitmap {
    uint32_t reserved;
} TextBitmap;

/* =========================
 * Global state
 * ========================= */
static jint       m_nSizeByte   = 0;
static jint       m_nWidText    = 0;
static jint       m_nHeiText    = 0;
static jint       m_nWidStrText = 0;
static jint       m_nHeiStrText = 0;
static jint       m_FontSize    = 12;
static jbyteArray m_TextParam   = NULL;

/* FreeType */
static FT_Library g_ft;
static FT_Face    g_face;

/* =========================
 * Utils
 * ========================= */
static int FixTextTextureSize(int n)
{
    if (n <= 16) return 16;
    int p = 32;
    while (p < n) p <<= 1;
    return p;
}
static FT_Library g_ft;

static FT_Face    g_face;

static bool       g_ft_init = false;

/* =========================

 * FIX: glyph cache (performance)

 * ========================= */

typedef struct {

    uint32_t code;

    int adv;

} GlyphCache;

static GlyphCache g_cache[1024];

static int g_cache_count = 0;

static int GetGlyphAdvance(uint32_t c)

{

    for (int i = 0; i < g_cache_count; i++) {

        if (g_cache[i].code == c)

            return g_cache[i].adv;

    }

    if (FT_Load_Char(g_face, c, FT_LOAD_DEFAULT))

        return 0;

    int adv = g_face->glyph->advance.x >> 6;

    if (g_cache_count < 1024) {

        g_cache[g_cache_count++] = (GlyphCache){c, adv};

    }

    return adv;

}

/* =========================

 * FIX: UTF-8 safe decode

 * ========================= */

static uint32_t* utf8_decode_safe(const char* s, int* out_len)

{

    int cap = strlen(s);

    uint32_t* buf = (uint32_t*)malloc(cap * sizeof(uint32_t));

    int i = 0;

    while (*s) {

        uint32_t c;

        unsigned char u = (unsigned char)*s;

        if (u < 0x80) {

            c = *s++;

        }

        else if ((u & 0xE0) == 0xC0 && s[1]) {

            c = ((u & 0x1F) << 6) | (s[1] & 0x3F);

            s += 2;

        }

        else if ((u & 0xF0) == 0xE0 && s[1] && s[2]) {

            c = ((u & 0x0F) << 12) |

                ((s[1] & 0x3F) << 6) |

                (s[2] & 0x3F);

            s += 3;

        }

        else {

            c = 0x3F;

            s++;

        }

        buf[i++] = c;

    }

    *out_len = i;

    return buf;

}

/* =========================

 * FIX: InitFont safe

 * ========================= */

static void InitFont()

{

    if (g_ft_init) return;

    if (FT_Init_FreeType(&g_ft)) {

        so_loge("FT init fail");

        return;

    }

    if (FT_New_Face(g_ft, TTF_PATH, 0, &g_face)) {

        so_loge("FT load fail");

        return;

    }

    if (FT_Select_Charmap(g_face, FT_ENCODING_UNICODE)) {

        so_loge("charmap fail");

    }

    FT_Set_Pixel_Sizes(g_face, 0, m_FontSize);

    g_ft_init = true;

}

/* =========================
 * Measure text (NO render!)
 * ========================= */
static int MeasureText(const char *text)
{
    int w = 0;

    for (const char *p = text; *p; p++) {

        /* ⚠️ 不 render，只取 advance */
        if (FT_Load_Char(g_face, *p, FT_LOAD_DEFAULT))
            continue;

        w += (g_face->glyph->advance.x >> 6);
    }

    return w;
}

/* =========================

 * FIX: FAST Measure

 * ========================= */

static int MeasureTextU32_fast(uint32_t* text, int len)

{

    int w = 0;

    for (int i = 0; i < len; i++) {

        w += GetGlyphAdvance(text[i]);

    }

    return w;

}

/* =========================

 * FIXED CreateTextBitmap

 * ========================= */

static jbyteArray CreateTextBitmap(jargs *args)

{

    const char *text = JniGetStringUTFChars(JARG(args, jstring), NULL);

    jint maxW = JARG(args, jint);

    InitFont();

    FT_Set_Pixel_Sizes(g_face, 0, m_FontSize);

    int ulen = 0;

    uint32_t* utext = utf8_decode_safe(text, &ulen);

    uint32_t* lines[256];

    int lineLens[256];

    int lineCount = 0;

    int start = 0;

    /* =========================

     * FIX: linear wrap (no O(n²) explosion)

     * ========================= */

    while (start < ulen && lineCount < 256) {

        int w = 0;

        int last = start;

        for (int i = start; i < ulen; i++) {

            int adv = GetGlyphAdvance(utext[i]);

            if (w + adv > maxW)

                break;

            w += adv;

            last = i + 1;

        }

        int len = last - start;

        if (len <= 0) len = 1;

        uint32_t* line = (uint32_t*)malloc(len * sizeof(uint32_t));

        memcpy(line, utext + start, len * sizeof(uint32_t));

        lines[lineCount] = line;

        lineLens[lineCount] = len;

        lineCount++;

        start = last;

    }

    free(utext);

    /* =========================

     * FIX metrics

     * ========================= */

    int lineHeight = (g_face->size->metrics.height >> 6);

    m_nWidStrText = maxW;

    m_nHeiStrText = lineHeight * lineCount;

    m_nWidText = FixTextTextureSize(m_nWidStrText);

    m_nHeiText = FixTextTextureSize(m_nHeiStrText);

    m_nSizeByte = m_nWidText * m_nHeiText * 4;

    uint32_t *img = (uint32_t*)calloc(m_nWidText * m_nHeiText, sizeof(uint32_t));

    /* =========================

     * DRAW SAFE

     * ========================= */

    for (int i = 0; i < lineCount; i++) {

        int x = 0;

        int y = (i + 1) * lineHeight;

        for (int j = 0; j < lineLens[i]; j++) {

            uint32_t c = lines[i][j];

            if (FT_Load_Char(g_face, c, FT_LOAD_RENDER))

                continue;

            FT_GlyphSlot g = g_face->glyph;

            FT_Bitmap *bmp = &g->bitmap;

            if (!bmp->buffer || bmp->rows == 0 || bmp->width == 0)

                continue;

            int gx = x + g->bitmap_left;

            int gy = y - g->bitmap_top;

            for (int r = 0; r < bmp->rows; r++) {

                int py = gy + r;

                if ((unsigned)py >= m_nHeiText) continue;

                for (int c2 = 0; c2 < bmp->width; c2++) {

                    int px = gx + c2;

                    if ((unsigned)px >= m_nWidText) continue;

                    uint8_t a = bmp->buffer[r * bmp->width + c2];

                    if (a == 0) continue;

                    img[py * m_nWidText + px] = (a << 24) | 0x00FFFFFF;

                }

            }

            x += (g->advance.x >> 6);

        }

    }

    jbyteArray ret = JniNewByteArray(m_nSizeByte);

    JniSetByteArrayRegion(ret, 0, m_nSizeByte, (jbyte*)img);

    for (int i = 0; i < lineCount; i++)

        free(lines[i]);

    free(img);

    return ret;

}

/* =========================
 * GetParam
 * ========================= */
static jbyteArray GetParam()
{
    if (!m_TextParam) {
        m_TextParam = JniNewGlobalRefTake(JniNewByteArray(20));
    }

    jbyte *b = JniGetByteArrayElements(m_TextParam, NULL);

    b[0] = m_nSizeByte;
    b[1] = m_nSizeByte >> 8;
    b[2] = m_nSizeByte >> 16;
    b[3] = m_nSizeByte >> 24;

    b[4] = m_nWidText;
    b[5] = m_nWidText >> 8;
    b[6] = m_nWidText >> 16;
    b[7] = m_nWidText >> 24;

    b[8]  = m_nHeiText;
    b[9]  = m_nHeiText >> 8;
    b[10] = m_nHeiText >> 16;
    b[11] = m_nHeiText >> 24;

    b[12] = m_nWidStrText;
    b[13] = m_nWidStrText >> 8;
    b[14] = m_nWidStrText >> 16;
    b[15] = m_nWidStrText >> 24;

    b[16] = m_nHeiStrText;
    b[17] = m_nHeiStrText >> 8;
    b[18] = m_nHeiStrText >> 16;
    b[19] = m_nHeiStrText >> 24;

    JniReleaseByteArrayElements(m_TextParam, b, 0);

    return JniNewLocalRef(m_TextParam);
}

/* =========================
 * SetupFontSize
 * ========================= */
static jint SetupFontSize(jargs *args)
{
    jint size = JARG(args, jint);
    m_FontSize = size;
    return m_FontSize;
}

/* =========================
 * JNI binding
 * ========================= */
static jsymbol class_symbols[] = {
        JNI_SYMBOL_STATIC_METHOD("CreateTextBitmap", "(Ljava/lang/String;II)[B", &CreateTextBitmap),
        JNI_SYMBOL_STATIC_METHOD("GetParam", "()[B", &GetParam),
        JNI_SYMBOL_STATIC_METHOD("SetupFontSize", "(I)I", &SetupFontSize),
};

static jclassMeta class_meta = {
        .name = "jp/co/capcom/android/bio4_LGUplus0119/TextBitmap",
        .super_name = NULL,
        .symbols = class_symbols,
        .num_symbols = sizeof(class_symbols) / sizeof(class_symbols[0]),
        .instance_size = sizeof(TextBitmap),
};

void TextBitmap_class_init()
{
    JniCreateClass(&class_meta);
}