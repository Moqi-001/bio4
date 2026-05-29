#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <pthread.h>

#include <psp2/io/fcntl.h>
#include <psp2/kernel/threadmgr.h>
#include <so_common/so_logger.h>
#include <so_jni/so_jni.h>

#include <ft2build.h>
#include FT_FREETYPE_H

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
static bool       g_ft_init = false;

static pthread_mutex_t g_freetype_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t g_cache_mutex = PTHREAD_MUTEX_INITIALIZER;

/* =========================
 * 完整的字形缓存（包含位图数据）
 * ========================= */
typedef struct {
    uint32_t code;
    int advance;
    int bitmap_left;
    int bitmap_top;
    int width;
    int rows;
    unsigned char* buffer;
} CachedGlyph;

#define GLYPH_CACHE_SIZE 512
static CachedGlyph g_glyph_cache[GLYPH_CACHE_SIZE];
static int g_glyph_cache_count = 0;

/* advance 快速查找表 */
static int g_advance_lut[128];
static bool g_advance_lut_init = false;

/* =========================
 * 文字纹理缓存
 * ========================= */
typedef struct {
    char* text;
    int maxW;
    int maxH;
    int fontSize;
    jbyteArray data;
    int sizeByte;
    int widText;
    int heiText;
    uint64_t lastAccess;
} CachedTexture;

// 缓存大小：32个纹理，PSV内存有限
#define TEXTURE_CACHE_SIZE 32
static CachedTexture g_texture_cache[TEXTURE_CACHE_SIZE];
static int g_texture_cache_count = 0;
static uint64_t g_cache_timestamp = 0;

/* =========================
 * Utils
 * ========================= */
static int FixTextTextureSize(int n) {
    if (n <= 16) return 16;
    int p = 32;
    while (p < n) p <<= 1;
    return p;
}

static uint32_t HashString(const char* s) {
    uint32_t hash = 5381;
    int c;
    while ((c = *s++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}

/* =========================
 * 缓存操作
 * ========================= */
static bool IsTextureEqual(CachedTexture* cache, const char* text, int maxW, int maxH, int fontSize) {
    if (cache->maxW != maxW || cache->maxH != maxH || cache->fontSize != fontSize) {
        return false;
    }
    if (!cache->text || !text) return cache->text == text;
    return strcmp(cache->text, text) == 0;
}

static jbyteArray GetCachedTexture(const char* text, int maxW, int maxH, int fontSize,
                                    int* outSizeByte, int* outWidText, int* outHeiText) {
    pthread_mutex_lock(&g_cache_mutex);

    for (int i = 0; i < g_texture_cache_count; i++) {
        if (IsTextureEqual(&g_texture_cache[i], text, maxW, maxH, fontSize)) {
            g_texture_cache[i].lastAccess = ++g_cache_timestamp;
            *outSizeByte = g_texture_cache[i].sizeByte;
            *outWidText = g_texture_cache[i].widText;
            *outHeiText = g_texture_cache[i].heiText;

            so_logd("[TextBitmap] Cache HIT: '%s'", text);
            jbyteArray result = JniNewLocalRef(g_texture_cache[i].data);
            pthread_mutex_unlock(&g_cache_mutex);
            return result;
        }
    }

    pthread_mutex_unlock(&g_cache_mutex);
    so_logd("[TextBitmap] Cache MISS: '%s'", text);
    return NULL;
}

static void CacheTexture(const char* text, int maxW, int maxH, int fontSize,
                         jbyteArray data, int sizeByte, int widText, int heiText) {
    pthread_mutex_lock(&g_cache_mutex);

    // LRU 淘汰：找到最旧的或者空位
    int replace_idx = -1;
    uint64_t oldest_time = ~0ULL;

    for (int i = 0; i < TEXTURE_CACHE_SIZE; i++) {
        if (i >= g_texture_cache_count) {
            replace_idx = i;
            break;
        }
        if (g_texture_cache[i].lastAccess < oldest_time) {
            oldest_time = g_texture_cache[i].lastAccess;
            replace_idx = i;
        }
    }

    // 释放旧缓存
    if (replace_idx >= 0 && replace_idx < g_texture_cache_count) {
        if (g_texture_cache[replace_idx].text) {
            free(g_texture_cache[replace_idx].text);
        }
        if (g_texture_cache[replace_idx].data) {
            JniDeleteGlobalRef(g_texture_cache[replace_idx].data);
        }
    } else if (replace_idx >= g_texture_cache_count) {
        g_texture_cache_count++;
    }

    // 存入新缓存
    g_texture_cache[replace_idx].text = strdup(text);
    g_texture_cache[replace_idx].maxW = maxW;
    g_texture_cache[replace_idx].maxH = maxH;
    g_texture_cache[replace_idx].fontSize = fontSize;
    g_texture_cache[replace_idx].data = JniNewGlobalRefTake(data);
    g_texture_cache[replace_idx].sizeByte = sizeByte;
    g_texture_cache[replace_idx].widText = widText;
    g_texture_cache[replace_idx].heiText = heiText;
    g_texture_cache[replace_idx].lastAccess = ++g_cache_timestamp;

    pthread_mutex_unlock(&g_cache_mutex);
}

static void ClearTextureCache() {
    pthread_mutex_lock(&g_cache_mutex);
    for (int i = 0; i < g_texture_cache_count; i++) {
        if (g_texture_cache[i].text) {
            free(g_texture_cache[i].text);
        }
        if (g_texture_cache[i].data) {
            JniDeleteGlobalRef(g_texture_cache[i].data);
        }
    }
    g_texture_cache_count = 0;
    pthread_mutex_unlock(&g_cache_mutex);
}

/* =========================
 * UTF-8 decode
 * ========================= */
static uint32_t* utf8_decode_safe(const char* s, int* out_len) {
    if (!s || !out_len) return NULL;

    int cap = strlen(s);
    uint32_t* buf = (uint32_t*)malloc(cap * sizeof(uint32_t));
    if (!buf) return NULL;

    int i = 0;
    while (*s && i < cap) {
        uint32_t c;
        unsigned char u = (unsigned char)*s;

        if (u < 0x80) {
            c = *s++;
        } else if ((u & 0xE0) == 0xC0 && s[1]) {
            c = ((u & 0x1F) << 6) | (s[1] & 0x3F);
            s += 2;
        } else if ((u & 0xF0) == 0xE0 && s[1] && s[2]) {
            c = ((u & 0x0F) << 12) | ((s[1] & 0x3F) << 6) | (s[2] & 0x3F);
            s += 3;
        } else if ((u & 0xF8) == 0xF0 && s[1] && s[2] && s[3]) {
            c = ((u & 0x07) << 18) | ((s[1] & 0x3F) << 12) | ((s[2] & 0x3F) << 6) | (s[3] & 0x3F);
            s += 4;
        } else {
            c = '?';
            s++;
        }
        buf[i++] = c;
    }

    *out_len = i;
    return buf;
}

/* =========================
 * Init FreeType
 * ========================= */
static void InitFont() {
    if (g_ft_init) return;

    pthread_mutex_lock(&g_freetype_mutex);

    if (!g_ft_init) {
        if (FT_Init_FreeType(&g_ft)) {
            so_loge("[TextBitmap] FT_Init_FreeType failed");
            pthread_mutex_unlock(&g_freetype_mutex);
            return;
        }

        if (FT_New_Face(g_ft, TTF_PATH, 0, &g_face)) {
            so_loge("[TextBitmap] FT_New_Face failed for path: %s", TTF_PATH);
            pthread_mutex_unlock(&g_freetype_mutex);
            return;
        }

        if (FT_Select_Charmap(g_face, FT_ENCODING_UNICODE)) {
            so_loge("[TextBitmap] FT_Select_Charmap failed");
        }

        FT_Set_Pixel_Sizes(g_face, 0, m_FontSize);
        g_ft_init = true;
        so_logd("[TextBitmap] Font initialized, size=%d", m_FontSize);
    }

    pthread_mutex_unlock(&g_freetype_mutex);
}

static void ClearGlyphCache() {
    pthread_mutex_lock(&g_freetype_mutex);
    for (int i = 0; i < g_glyph_cache_count; i++) {
        if (g_glyph_cache[i].buffer) {
            free(g_glyph_cache[i].buffer);
            g_glyph_cache[i].buffer = NULL;
        }
    }
    g_glyph_cache_count = 0;
    g_advance_lut_init = false;
    pthread_mutex_unlock(&g_freetype_mutex);
}

static CachedGlyph* GetOrLoadGlyph(uint32_t c) {
    pthread_mutex_lock(&g_freetype_mutex);

    // 检查缓存
    for (int i = 0; i < g_glyph_cache_count; i++) {
        if (g_glyph_cache[i].code == c) {
            pthread_mutex_unlock(&g_freetype_mutex);
            return &g_glyph_cache[i];
        }
    }

    // 缓存满了就清空（简化 LRU）
    if (g_glyph_cache_count >= GLYPH_CACHE_SIZE) {
        for (int i = 0; i < g_glyph_cache_count; i++) {
            if (g_glyph_cache[i].buffer) {
                free(g_glyph_cache[i].buffer);
                g_glyph_cache[i].buffer = NULL;
            }
        }
        g_glyph_cache_count = 0;
        g_advance_lut_init = false;
    }

    // 加载字形并渲染
    if (FT_Load_Char(g_face, c, FT_LOAD_RENDER)) {
        pthread_mutex_unlock(&g_freetype_mutex);
        return NULL;
    }

    FT_GlyphSlot slot = g_face->glyph;
    FT_Bitmap* bmp = &slot->bitmap;

    // 复制位图数据
    unsigned char* buffer_copy = NULL;
    if (bmp->buffer && bmp->rows > 0 && bmp->width > 0) {
        buffer_copy = (unsigned char*)malloc(bmp->rows * bmp->width);
        if (buffer_copy) {
            memcpy(buffer_copy, bmp->buffer, bmp->rows * bmp->width);
        }
    }

    int idx = g_glyph_cache_count++;
    g_glyph_cache[idx].code = c;
    g_glyph_cache[idx].advance = slot->advance.x >> 6;
    if (g_glyph_cache[idx].advance <= 0) g_glyph_cache[idx].advance = m_FontSize / 2;
    g_glyph_cache[idx].bitmap_left = slot->bitmap_left;
    g_glyph_cache[idx].bitmap_top = slot->bitmap_top;
    g_glyph_cache[idx].width = bmp->width;
    g_glyph_cache[idx].rows = bmp->rows;
    g_glyph_cache[idx].buffer = buffer_copy;

    // ASCII 快速路径
    if (c < 128) {
        g_advance_lut[c] = g_glyph_cache[idx].advance;
        if (c == 0) g_advance_lut_init = true;
    }

    pthread_mutex_unlock(&g_freetype_mutex);
    return &g_glyph_cache[idx];
}

static int GetGlyphAdvance(uint32_t c) {
    pthread_mutex_lock(&g_freetype_mutex);

    // ASCII 快速路径
    if (c < 128 && g_advance_lut_init) {
        int adv = g_advance_lut[c];
        if (adv > 0) {
            pthread_mutex_unlock(&g_freetype_mutex);
            return adv;
        }
    }

    // 检查缓存
    for (int i = 0; i < g_glyph_cache_count; i++) {
        if (g_glyph_cache[i].code == c) {
            int adv = g_glyph_cache[i].advance;
            pthread_mutex_unlock(&g_freetype_mutex);
            return adv;
        }
    }

    // 临时加载（不渲染位图）
    if (FT_Load_Char(g_face, c, FT_LOAD_DEFAULT)) {
        pthread_mutex_unlock(&g_freetype_mutex);
        return m_FontSize / 2;
    }

    int adv = g_face->glyph->advance.x >> 6;
    if (adv <= 0) adv = m_FontSize / 2;

    pthread_mutex_unlock(&g_freetype_mutex);
    return adv;
}

static void RenderChar(uint32_t c, int x, int y, uint32_t* buffer, int bufferWidth, int bufferHeight) {
    CachedGlyph* glyph = GetOrLoadGlyph(c);
    if (!glyph || !glyph->buffer || glyph->rows == 0 || glyph->width == 0) return;

    int left = glyph->bitmap_left;
    int top = glyph->bitmap_top;

    // 预计算边界
    int start_row = y - top;
    if (start_row < 0) start_row = 0;
    int end_row = y - top + glyph->rows;
    if (end_row > bufferHeight) end_row = bufferHeight;

    int start_col = x + left;
    if (start_col < 0) start_col = 0;
    int end_col = x + left + glyph->width;
    if (end_col > bufferWidth) end_col = bufferWidth;

    // 渲染
    for (int row = start_row; row < end_row; row++) {
        int src_row = row - (y - top);
        if (src_row < 0 || src_row >= glyph->rows) continue;

        uint32_t* dst_row = &buffer[row * bufferWidth];
        unsigned char* src_row_data = &glyph->buffer[src_row * glyph->width];

        for (int col = start_col; col < end_col; col++) {
            int src_col = col - (x + left);
            if (src_col < 0 || src_col >= glyph->width) continue;

            unsigned char alpha = src_row_data[src_col];
            if (alpha == 0) continue;

            dst_row[col] = (alpha << 24) | 0x00FFFFFF;
        }
    }
}

/* =========================
 * 分行逻辑
 * ========================= */
typedef struct {
    uint32_t* chars;
    int length;
    int width;
} TextLine;

static TextLine* SplitTextToLines(uint32_t* text, int len, int maxWidth, int* lineCount) {
    if (!text || len <= 0 || !lineCount) return NULL;

    TextLine* lines = (TextLine*)malloc(sizeof(TextLine) * 256);
    if (!lines) return NULL;

    *lineCount = 0;
    int start = 0;

    // 预计算所有 advance
    int* advances = (int*)malloc(len * sizeof(int));
    if (!advances) {
        free(lines);
        return NULL;
    }

    for (int i = 0; i < len; i++) {
        advances[i] = GetGlyphAdvance(text[i]);
    }

    while (start < len && *lineCount < 256) {
        int lineWidth = 0;
        int lastChar = start;

        for (int i = start; i < len; i++) {
            if (lineWidth + advances[i] > maxWidth && lineWidth > 0) {
                break;
            }
            lineWidth += advances[i];
            lastChar = i + 1;
        }

        if (lastChar == start) {
            lastChar = start + 1;
            lineWidth = advances[start];
        }

        int lineLen = lastChar - start;
        uint32_t* lineChars = (uint32_t*)malloc(lineLen * sizeof(uint32_t));
        if (!lineChars) {
            for (int j = 0; j < *lineCount; j++) free(lines[j].chars);
            free(lines);
            free(advances);
            return NULL;
        }

        memcpy(lineChars, text + start, lineLen * sizeof(uint32_t));

        lines[*lineCount].chars = lineChars;
        lines[*lineCount].length = lineLen;
        lines[*lineCount].width = lineWidth;
        (*lineCount)++;

        start = lastChar;
    }

    free(advances);
    return lines;
}

/* =========================
 * 创建纹理（带缓存）
 * ========================= */
static jbyteArray CreateTextBitmap(jargs *args) {
    const char *text = JniGetStringUTFChars(JARG(args, jstring), NULL);
    jint maxW = JARG(args, jint);
    jint maxH = JARG(args, jint);

    if (!text) {
        so_loge("[TextBitmap] CreateTextBitmap: text is NULL");
        return NULL;
    }

    so_logd("[TextBitmap] CreateTextBitmap: text='%s', maxW=%d, maxH=%d, fontSize=%d",
            text, maxW, maxH, m_FontSize);

    // 先查缓存
    int cachedSizeByte = 0, cachedWidText = 0, cachedHeiText = 0;
    jbyteArray cached = GetCachedTexture(text, maxW, maxH, m_FontSize,
                                          &cachedSizeByte, &cachedWidText, &cachedHeiText);
    if (cached) {
        m_nSizeByte = cachedSizeByte;
        m_nWidText = cachedWidText;
        m_nHeiText = cachedHeiText;
        m_nWidStrText = cachedWidText;
        m_nHeiStrText = cachedHeiText;
        return cached;
    }

    // 初始化字体
    InitFont();
    if (!g_ft_init) {
        so_loge("[TextBitmap] Font not initialized");
        return NULL;
    }

    FT_Set_Pixel_Sizes(g_face, 0, m_FontSize);

    // UTF-8 解码
    int ulen = 0;
    uint32_t* utext = utf8_decode_safe(text, &ulen);
    if (!utext || ulen == 0) {
        if (utext) free(utext);
        return NULL;
    }

    // 分行
    int lineCount = 0;
    TextLine* lines = SplitTextToLines(utext, ulen, maxW, &lineCount);
    free(utext);

    if (!lines || lineCount == 0) {
        if (lines) free(lines);
        return NULL;
    }

    // 计算尺寸
    int lineHeight = (g_face->size->metrics.height >> 6);
    int baseline = (g_face->size->metrics.ascender >> 6);
    if (lineHeight <= 0) lineHeight = m_FontSize;
    if (baseline <= 0) baseline = m_FontSize;

    int maxTextWidth = 0;
    for (int i = 0; i < lineCount; i++) {
        if (lines[i].width > maxTextWidth) maxTextWidth = lines[i].width;
    }
    if (maxTextWidth > maxW) maxTextWidth = maxW;
    if (maxTextWidth <= 0) maxTextWidth = m_FontSize * 2;

    int totalTextHeight = lineHeight * lineCount;
    if (totalTextHeight <= 0) totalTextHeight = m_FontSize;
    if (totalTextHeight > maxH) {
        totalTextHeight = maxH;
        lineCount = totalTextHeight / lineHeight;
        if (lineCount <= 0) lineCount = 1;
    }

    // 纹理尺寸（2的幂）
    int texWidth = FixTextTextureSize(maxTextWidth);
    int texHeight = FixTextTextureSize(totalTextHeight);
    int sizeByte = texWidth * texHeight;

    // 分配并渲染
    uint32_t* img = (uint32_t*)calloc(texWidth * texHeight, sizeof(uint32_t));
    if (!img) {
        for (int i = 0; i < lineCount; i++) free(lines[i].chars);
        free(lines);
        return NULL;
    }

    for (int i = 0; i < lineCount; i++) {
        int x = 0;
        int y = baseline + (i * lineHeight);
        for (int j = 0; j < lines[i].length; j++) {
            RenderChar(lines[i].chars[j], x, y, img, texWidth, texHeight);
            x += GetGlyphAdvance(lines[i].chars[j]);
        }
    }

    // 转换为字节数组（只取 Alpha）
    jbyteArray result = JniNewByteArray(sizeByte);
    if (result) {
        jbyte* byteData = JniGetByteArrayElements(result, NULL);
        if (byteData) {
            for (int i = 0; i < sizeByte; i++) {
                byteData[i] = (jbyte)((img[i] >> 24) & 0xFF);
            }
            JniReleaseByteArrayElements(result, byteData, 0);
        }
    }

    // 清理
    free(img);
    for (int i = 0; i < lineCount; i++) {
        free(lines[i].chars);
    }
    free(lines);

    // 更新全局状态并缓存
    m_nSizeByte = sizeByte;
    m_nWidText = texWidth;
    m_nHeiText = texHeight;
    m_nWidStrText = maxTextWidth;
    m_nHeiStrText = totalTextHeight;

    CacheTexture(text, maxW, maxH, m_FontSize, result, sizeByte, texWidth, texHeight);

    so_logd("[TextBitmap] Created: %dx%d, size=%d, lines=%d", texWidth, texHeight, sizeByte, lineCount);

    return result;
}

/* =========================
 * 其他 JNI 函数
 * ========================= */
static jbyteArray GetParam(jargs *args) {
    (void)args;

    if (!m_TextParam) {
        m_TextParam = JniNewGlobalRefTake(JniNewByteArray(20));
        if (!m_TextParam) return NULL;
    }

    jbyte *b = JniGetByteArrayElements(m_TextParam, NULL);
    if (!b) return JniNewLocalRef(m_TextParam);

    b[0]  = (jbyte)((m_nSizeByte >> 0) & 0xFF);
    b[1]  = (jbyte)((m_nSizeByte >> 8) & 0xFF);
    b[2]  = (jbyte)((m_nSizeByte >> 16) & 0xFF);
    b[3]  = (jbyte)((m_nSizeByte >> 24) & 0xFF);
    b[4]  = (jbyte)((m_nWidText >> 0) & 0xFF);
    b[5]  = (jbyte)((m_nWidText >> 8) & 0xFF);
    b[6]  = (jbyte)((m_nWidText >> 16) & 0xFF);
    b[7]  = (jbyte)((m_nWidText >> 24) & 0xFF);
    b[8]  = (jbyte)((m_nHeiText >> 0) & 0xFF);
    b[9]  = (jbyte)((m_nHeiText >> 8) & 0xFF);
    b[10] = (jbyte)((m_nHeiText >> 16) & 0xFF);
    b[11] = (jbyte)((m_nHeiText >> 24) & 0xFF);
    b[12] = (jbyte)((m_nWidStrText >> 0) & 0xFF);
    b[13] = (jbyte)((m_nWidStrText >> 8) & 0xFF);
    b[14] = (jbyte)((m_nWidStrText >> 16) & 0xFF);
    b[15] = (jbyte)((m_nWidStrText >> 24) & 0xFF);
    b[16] = (jbyte)((m_nHeiStrText >> 0) & 0xFF);
    b[17] = (jbyte)((m_nHeiStrText >> 8) & 0xFF);
    b[18] = (jbyte)((m_nHeiStrText >> 16) & 0xFF);
    b[19] = (jbyte)((m_nHeiStrText >> 24) & 0xFF);

    JniReleaseByteArrayElements(m_TextParam, b, 0);
    return JniNewLocalRef(m_TextParam);
}

static jint GetSize(jargs *args) { (void)args; return m_nSizeByte; }
static jint GetWidth(jargs *args) { (void)args; return m_nWidText; }
static jint GetHeight(jargs *args) { (void)args; return m_nHeiText; }
static jint GetStrWidth(jargs *args) { (void)args; return m_nWidStrText; }
static jint GetStrHeight(jargs *args) { (void)args; return m_nHeiStrText; }

static jint SetupFontSize(jargs *args) {
    jint targetSize = JARG(args, jint);

    InitFont();
    if (!g_ft_init) {
        m_FontSize = 12;
        return m_FontSize;
    }

    int oldSize = m_FontSize;
    m_FontSize = 12;
    for (int i = 50; i >= 12; i -= 2) {
        FT_Set_Pixel_Sizes(g_face, 0, i);
        int fontHeight = (g_face->size->metrics.height >> 6);
        if (targetSize >= fontHeight) {
            m_FontSize = i;
            break;
        }
    }
    FT_Set_Pixel_Sizes(g_face, 0, m_FontSize);

    // 字体大小改变，清空所有缓存
    if (oldSize != m_FontSize) {
        ClearTextureCache();
        ClearGlyphCache();
        so_logd("[TextBitmap] Font size changed, caches cleared");
    }

    so_logd("[TextBitmap] SetupFontSize: target=%d, result=%d", targetSize, m_FontSize);
    return m_FontSize;
}

static jbyteArray getByte(jargs *args) {
    (void)args;
    jbyteArray result = JniNewByteArray(4);
    if (result) {
        jbyte* bytes = JniGetByteArrayElements(result, NULL);
        if (bytes) {
            bytes[0] = 1; bytes[1] = 2; bytes[2] = 34; bytes[3] = 4;
            JniReleaseByteArrayElements(result, bytes, 0);
        }
    }
    return result;
}

/* =========================
 * JNI binding
 * ========================= */
static jsymbol class_symbols[] = {
    JNI_SYMBOL_STATIC_METHOD("CreateTextBitmap", "(Ljava/lang/String;II)[B", &CreateTextBitmap),
    JNI_SYMBOL_STATIC_METHOD("GetParam", "()[B", &GetParam),
    JNI_SYMBOL_STATIC_METHOD("GetSize", "()I", &GetSize),
    JNI_SYMBOL_STATIC_METHOD("GetWidth", "()I", &GetWidth),
    JNI_SYMBOL_STATIC_METHOD("GetHeight", "()I", &GetHeight),
    JNI_SYMBOL_STATIC_METHOD("GetStrWidth", "()I", &GetStrWidth),
    JNI_SYMBOL_STATIC_METHOD("GetStrHeight", "()I", &GetStrHeight),
    JNI_SYMBOL_STATIC_METHOD("SetupFontSize", "(I)I", &SetupFontSize),
    JNI_SYMBOL_STATIC_METHOD("getByte", "()[B", &getByte),
};

static jclassMeta class_meta = {
    .name = "jp/co/capcom/android/bio4_LGUplus0119/TextBitmap",
    .super_name = NULL,
    .symbols = class_symbols,
    .num_symbols = sizeof(class_symbols) / sizeof(class_symbols[0]),
    .instance_size = sizeof(TextBitmap),
};

void TextBitmap_class_init() {
    JniCreateClass(&class_meta);
}