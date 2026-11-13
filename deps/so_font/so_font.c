#include <stdlib.h>
#include <string.h>

#include <psp2/kernel/threadmgr.h>

#include <freetype2/ft2build.h>
#include FT_FREETYPE_H

#include "font_atlas.h"

#define DEFAULT_ATLAS_W 512
#define DEFAULT_ATLAS_H 512

typedef struct so_font {
    void                *font_buffer;
    uint32_t             buffer_size;
    FT_Library           ftlibrary;
    FT_Face              ftface;
    font_atlas         **atlases;
    int                  num_atlases;
    SceKernelLwMutexWork mutex;
} so_font;

typedef struct text_line {
    const font_glyph **glyphs;
    int                num_glyphs;
} text_line;

typedef struct text_info {
    text_line         *lines;
    const font_glyph **glyphs;
    int                num_lines;
    int                num_glyphs;
} text_info;

static int utf8_to_ucs4(const unsigned char *utf8, uint32_t *codepoint) {
    if ((utf8[0] & 0x80) == 0) {
        *codepoint = *utf8;
        return 1;
    }
    if ((utf8[0] & 0xE0) == 0xC0) {
        *codepoint = ((utf8[0] & 0x1F) << 6) | (utf8[1] & 0x3F);
        return 2;
    }
    if ((utf8[0] & 0xF0) == 0xE0) {
        *codepoint = ((utf8[0] & 0x0F) << 12) | ((utf8[1] & 0x3F) << 6) | (utf8[2] & 0x3F);
        return 3;
    }
    if ((utf8[0] & 0xF8) == 0xF0) {
        *codepoint = ((utf8[0] & 0x07) << 18) | ((utf8[1] & 0x3F) << 12) | ((utf8[2] & 0x3F) << 6) | (utf8[3] & 0x3F);
        return 4;
    }

    *codepoint = 0xFFFD;  // 无效时输出"�"
    return 1;
}

so_font *so_font_load_mem(void *buffer, size_t buffer_size, int take) {
    if (!buffer || buffer_size == 0) return NULL;

    so_font *font = calloc(1, sizeof(*font));
    if (!font)
        return NULL;

    if (take) {
        font->font_buffer = buffer;
    } else {
        font->font_buffer = malloc(buffer_size);
        if (!font->font_buffer) {
            free(font);
            return NULL;
        }
        memcpy(font->font_buffer, buffer, buffer_size);
    }
    font->buffer_size = buffer_size;

    FT_Error error;
    error = FT_Init_FreeType(&font->ftlibrary);
    if (error != FT_Err_Ok) {
        so_font_unload(font);
        return NULL;
    }

    error = FT_New_Memory_Face(
        font->ftlibrary,
        font->font_buffer,
        font->buffer_size,
        0,
        &font->ftface);
    if (error != FT_Err_Ok) {
        so_font_unload(font);
        return NULL;
    }

    error = FT_Select_Charmap(font->ftface, FT_ENCODING_UNICODE);
    if (error != FT_Err_Ok) {
        so_font_unload(font);
        return NULL;
    }

    int num_atlases = 4;  // 初始化4个插槽（仅指针，实际图集内存在需要时才分配，每集图集对应字体大小）
    font->atlases   = calloc(num_atlases, sizeof(font_atlas *));
    if (!font->atlases) {
        so_font_unload(font);
        return NULL;
    }
    font->num_atlases = num_atlases;

    sceKernelCreateLwMutex(&font->mutex, "so_font_mutex", 2, 0, NULL);
    return font;
}

so_font *so_font_load(const char *filepath) {
    FILE *fp = fopen(filepath, "rb");
    if (!fp) {
        return NULL;
    }

    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    if (size <= 0) {
        fclose(fp);
        return NULL;
    }

    void *buffer = malloc(size);
    if (!buffer) {
        fclose(fp);
        return NULL;
    }

    fseek(fp, 0, SEEK_SET);
    size_t read = fread(buffer, 1, size, fp);
    fclose(fp);

    if (read != size) {
        free(buffer);
        return NULL;
    }

    so_font *font = so_font_load_mem(buffer, size, 1);
    if (!font) {  // 移交给 load_mem，若失败则由 load_mem 负责释放 buffer
        return NULL;
    }

    return font;
}

void so_font_unload(so_font *font) {
    if (!font) return;

    if (font->ftface) {
        FT_Done_Face(font->ftface);
    }
    if (font->ftlibrary) {
        FT_Done_FreeType(font->ftlibrary);
    }
    if (font->font_buffer) {
        free(font->font_buffer);
    }
    if (font->atlases) {
        for (int i = 0; i < font->num_atlases; i++) {
            if (font->atlases[i]) {
                font_atlas_free(font->atlases[i]);
            }
        }
        free(font->atlases);
    }
    free(font);
}

static font_atlas *_font_get_atlas(so_font *font, uint32_t font_size) {
    int index;

    // 查找缓存图集
    for (index = 0; index < font->num_atlases; index++) {
        if (!font->atlases[index]) {  // 图集按顺序创建的，若无则表示后面也无，直接break
            break;
        }
        if (font->atlases[index]->font_size == font_size) {  // 匹配字体大小
            return font->atlases[index];
        }
    }

    // 若当前列表已全部在使用且未找到想要的图集，进行扩容，每次加4个插槽
    if (index >= font->num_atlases) {
        int          old_num_atlases = font->num_atlases;
        int          new_num_atlases = old_num_atlases + 4;
        font_atlas **new_atlases     = realloc(font->atlases, new_num_atlases * sizeof(font_atlas *));
        if (!new_atlases) {
            return NULL;
        }
        memset(new_atlases + old_num_atlases, 0, 4 * sizeof(font_atlas *));
        font->atlases     = new_atlases;
        font->num_atlases = new_num_atlases;
    }

    // 当前位置图集未使用过，需初始化
    if (!font->atlases[index]) {
        font->atlases[index] = font_atlas_new(font_size, DEFAULT_ATLAS_W, DEFAULT_ATLAS_H);
        if (!font->atlases[index]) {
            return NULL;
        }

        if (FT_Set_Pixel_Sizes(font->ftface, 0, font_size) != FT_Err_Ok) {
            font_atlas_free(font->atlases[index]);
            font->atlases[index] = NULL;
            return NULL;
        }

        font->atlases[index]->font_size      = font_size;
        font->atlases[index]->font_height    = font->ftface->size->metrics.height >> 6;
        font->atlases[index]->font_ascender  = font->ftface->size->metrics.ascender >> 6;
        font->atlases[index]->font_descender = font->ftface->size->metrics.descender >> 6;
        if (font->atlases[index]->font_height < (font->atlases[index]->font_ascender - font->atlases[index]->font_descender)) {
            font->atlases[index]->font_height = (font->atlases[index]->font_ascender - font->atlases[index]->font_descender);
        }
    }

    return font->atlases[index];
}

static const font_glyph *_font_add_glyph(so_font *font, font_atlas *atlas, uint32_t codepoint) {
    if (FT_Set_Pixel_Sizes(font->ftface, 0, atlas->font_size) != FT_Err_Ok) {
        return NULL;
    }
    if (FT_Load_Char(font->ftface, codepoint, FT_LOAD_TARGET_NORMAL)) {
        return NULL;
    }
    if (FT_Render_Glyph(font->ftface->glyph, FT_RENDER_MODE_NORMAL)) {
        return NULL;
    }

    const font_glyph *new_glyph = font_atlas_insert(atlas, codepoint, font->ftface->glyph);
    if (!new_glyph) {
        return NULL;
    }

    return new_glyph;
}

static const font_glyph *_font_get_glyph(so_font *font, font_atlas *atlas, uint32_t codepoint) {
    const font_glyph *glyph = font_atlas_find(atlas, codepoint);
    if (glyph) {
        return glyph;
    }

    glyph = _font_add_glyph(font, atlas, codepoint);
    if (!glyph && codepoint != 0xFFFD) {
        glyph = _font_get_glyph(font, atlas, 0xFFFD);  // 未找到，输出"�"
    }

    return glyph;
}

static text_info *_font_create_text_info(so_font *font, font_atlas *atlas, const char *text, int text_len) {
    const unsigned char *p          = (unsigned char *)text;
    const unsigned char *end        = (unsigned char *)text + text_len;
    int                  num_lines  = 1;
    int                  num_glyphs = 0;

    while (p < end) {
        if (*p == '\n') {
            p++;
            num_lines++;
            continue;
        }
        if (*p == '\r') {
            p++;
            if (*p == '\n') {
                p++;
                num_lines++;
            }
            continue;
        }
        if ((*p & 0x80) == 0) {
            p += 1;
        } else if ((*p & 0xE0) == 0xC0) {
            p += 2;
        } else if ((*p & 0xF0) == 0xE0) {
            p += 3;
        } else if ((*p & 0xF8) == 0xF0) {
            p += 4;
        } else {
            p += 1;
        }

        num_glyphs++;
    }

    text_info *info = calloc(1, sizeof(text_info));
    if (!info) {
        return NULL;
    }
    text_line *lines = calloc(num_lines, sizeof(text_line));
    if (!lines) {
        free(info);
        return NULL;
    }
    const font_glyph **glyphs = calloc(num_glyphs, sizeof(font_glyph *));
    if (!glyphs) {
        free(lines);
        free(info);
        return NULL;
    }

    text_line *line;
    uint32_t   codepoint;
    int        line_idx  = 0;
    int        glyph_idx = 0;
    line                 = &lines[line_idx];
    line->glyphs         = &glyphs[glyph_idx];
    line->num_glyphs     = 0;

    p = (unsigned char *)text;
    while (p < end) {
        if (*p == '\n') {
            p++;
        break_line:
            if (line_idx >= num_lines) {
                break;
            }
            line_idx++;
            line         = &lines[line_idx];
            line->glyphs = &glyphs[glyph_idx];
            continue;
        }
        if (*p == '\r') {
            p++;
            if (*p == '\n') {
                p++;
                goto break_line;
            }
            continue;
        }
        p += utf8_to_ucs4(p, &codepoint);
        if (glyph_idx >= num_glyphs) {
            break;
        }
        line->glyphs[line->num_glyphs] = _font_get_glyph(font, atlas, codepoint);
        if (line->glyphs[line->num_glyphs]) {
            line->num_glyphs++;
            glyph_idx++;
        }
    }
    line_idx++;

    info->lines      = lines;
    info->glyphs     = glyphs;
    info->num_lines  = line_idx;
    info->num_glyphs = glyph_idx;

    return info;
}

static void _font_free_text_info(text_info *info) {
    if (!info) return;

    if (info->lines) free(info->lines);
    if (info->glyphs) free(info->glyphs);
    free(info);
}

/* 测量文字位图，需保证缓冲区能盖住所有墨迹 */
static void _font_measure_text_bitmap(text_info *info, const font_atlas *atlas, int line_height,
                                      int *out_x0, int *out_y0, int *out_x1, int *out_y1) {
    int ink_x0 = 0, ink_y0 = 0, ink_x1 = 0, ink_y1 = 0;
    int pen_y = 0;

    for (int l = 0; l < info->num_lines; l++) {
        text_line *line  = &info->lines[l];
        int        pen_x = 0;

        for (int x = 0; x < line->num_glyphs; x++) {
            const font_glyph *glyph = line->glyphs[x];
            if (!glyph) {
                continue;
            }

            int sx = pen_x + glyph->bitmap_left;
            int sy = pen_y + (atlas->font_ascender - glyph->bitmap_top);
            int dx = sx + glyph->bitmap_width;
            int dy = sy + glyph->bitmap_height;
            if (sx < ink_x0) ink_x0 = sx;
            if (sy < ink_y0) ink_y0 = sy;
            if (dx > ink_x1) ink_x1 = dx;
            if (dy > ink_y1) ink_y1 = dy;

            pen_x += glyph->advance_x;
        }

        pen_y += line_height;
    }

    *out_x0 = ink_x0;
    *out_y0 = ink_y0;
    *out_x1 = ink_x1;
    *out_y1 = ink_y1;
}

so_bitmap *so_font_create_text_bitmap(so_font *font, const char *text, int font_size, int linespace) {
    if (!font || !text || font_size <= 0) return NULL;

    sceKernelLockLwMutex(&font->mutex, 1, NULL);

    font_atlas *atlas = _font_get_atlas(font, font_size);
    if (!atlas) {
        sceKernelUnlockLwMutex(&font->mutex, 1);
        return NULL;
    }

    int        text_len = strlen(text);
    text_info *info     = _font_create_text_info(font, atlas, text, text_len);
    if (!info) {
        sceKernelUnlockLwMutex(&font->mutex, 1);
        return NULL;
    }

    sceKernelUnlockLwMutex(&font->mutex, 1);

    int line_height = atlas->font_height + linespace;

    int ink_x0, ink_y0, ink_x1, ink_y1;
    _font_measure_text_bitmap(info, atlas, line_height, &ink_x0, &ink_y0, &ink_x1, &ink_y1);

    so_bitmap *bmp = calloc(1, sizeof(*bmp));
    if (!bmp) {
        _font_free_text_info(info);
        return NULL;
    }

    bmp->width    = ink_x1 - ink_x0;
    bmp->height   = ink_y1 - ink_y0;
    bmp->pitch    = bmp->width;
    bmp->origin_x = -ink_x0;
    bmp->origin_y = -ink_y0;

    if (bmp->width <= 0 || bmp->height <= 0) {
        bmp->pixels = malloc(1);  // 分配至少 1 字节，用以兼容空文本
    } else {
        bmp->pixels = calloc(bmp->pitch, bmp->height);
    }
    if (!bmp->pixels) {
        free(bmp);
        _font_free_text_info(info);
        return NULL;
    }

    int pen_y = bmp->origin_y;

    for (int l = 0; l < info->num_lines; l++) {
        text_line *line  = &info->lines[l];
        int        pen_x = bmp->origin_x;

        for (int i = 0; i < line->num_glyphs; i++) {
            const font_glyph *glyph = line->glyphs[i];
            if (!glyph) {
                continue;
            }

            int dst_x  = pen_x + glyph->bitmap_left;
            int dst_y  = pen_y + (atlas->font_ascender - glyph->bitmap_top);
            int src_x  = 0;
            int src_y  = 0;
            int copy_w = glyph->bitmap_width;
            int copy_h = glyph->bitmap_height;

            if (dst_x < 0) {
                src_x = -dst_x;
                copy_w -= src_x;
                dst_x = 0;
            }
            if (dst_y < 0) {
                src_y = -dst_y;
                copy_h -= src_y;
                dst_y = 0;
            }
            if (dst_x + copy_w > bmp->width) {
                copy_w = bmp->width - dst_x;
            }
            if (dst_y + copy_h > bmp->height) {
                copy_h = bmp->height - dst_y;
            }
            if (copy_w <= 0 || copy_h <= 0) {
                pen_x += glyph->advance_x;
                continue;
            }

            const unsigned char *src = glyph->bitmap_buffer + src_x + src_y * glyph->bitmap_pitch;
            unsigned char       *dst = bmp->pixels + dst_x + dst_y * bmp->pitch;
            for (int y = 0; y < copy_h; y++) {
                memcpy(dst, src, copy_w);
                src += glyph->bitmap_pitch;
                dst += bmp->pitch;
            }

            pen_x += glyph->advance_x;
        }

        pen_y += line_height;
    }

    _font_free_text_info(info);
    return bmp;
}

void so_font_free_bitmap(so_bitmap *bmp) {
    if (!bmp) return;
    if (bmp->pixels) free(bmp->pixels);
    free(bmp);
}

size_t so_font_get_font_height(so_font *font, size_t font_size) {
    if (!font) return 0;

    size_t height = 0;
    sceKernelLockLwMutex(&font->mutex, 1, NULL);
    if (FT_Set_Pixel_Sizes(font->ftface, 0, font_size) == FT_Err_Ok) {
        height = (font->ftface->size->metrics.height >> 6);
    }
    sceKernelUnlockLwMutex(&font->mutex, 1);

    return height;
}
