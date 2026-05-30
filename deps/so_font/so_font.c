#include <stdlib.h>
#include <string.h>

#include <freetype2/ft2build.h>
#include FT_FREETYPE_H

#include "font_atlas.h"

#define DEFAULT_ATLAS_W 512
#define DEFAULT_ATLAS_H 512

typedef struct so_font {
    void        *font_buffer;
    uint32_t     buffer_size;
    FT_Library   ftlibrary;
    FT_Face      ftface;
    font_atlas **atlases;
    int          num_atlases;
} so_font;

typedef struct text_line {
    const font_glyph **glyphs;
    int                num_chars;
    int                line_width;
} text_line;

typedef struct text_info {
    text_line         *lines;
    const font_glyph **glyphs;
    int                num_lines;
    int                num_chars;
    int                max_width;
} text_info;

static int utf8_to_ucs4_length(const unsigned char *utf8, int utf8_len) {
    if (!utf8 || utf8_len <= 0) return 0;

    int                  ucs4_len = 0;
    const unsigned char *p        = (const unsigned char *)utf8;
    const unsigned char *end      = p + utf8_len;

    while (p < end) {
        if ((utf8[0] & 0x80) == 0) {  // 1字节 ASCII
            p += 1;
        } else if ((*p & 0xE0) == 0xC0) {  // 2字节 UTF-8
            p += 2;
        } else if ((*p & 0xF0) == 0xE0) {  // 3字节 UTF-8
            p += 3;
        } else if ((*p & 0xF8) == 0xF0) {  // 4字节 UTF-8
            p += 4;
        } else {  // 非法 UTF-8
            p += 1;
        }
        ucs4_len++;
    }

    return ucs4_len;
}

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

so_font *so_font_load_mem(void *buffer, size_t buffer_size, int copy) {
    if (!buffer || buffer_size == 0) return NULL;

    so_font *font = calloc(1, sizeof(*font));
    if (!font)
        return NULL;

    if (copy) {
        font->font_buffer = malloc(buffer_size);
        if (!font->font_buffer) {
            free(font);
            return NULL;
        }
        memcpy(font->font_buffer, buffer, buffer_size);
    } else {
        font->font_buffer = buffer;
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

    font->num_atlases = 4;  // 初始化4个插槽（仅指针，实际图集内存在需要时才分配，每集图集对应字体大小）
    font->atlases     = calloc(2, sizeof(font_atlas *));
    if (!font->atlases) {
        so_font_unload(font);
        return NULL;
    }

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

    so_font *font = so_font_load_mem(buffer, size, 0);
    if (!font) {
        free(buffer);
        return NULL;
    }

    return font;
}

void so_font_unload(so_font *font) {
    if (!font) return;

    if (font->font_buffer) {
        free(font->font_buffer);
    }
    if (font->ftface) {
        FT_Done_Face(font->ftface);
    }
    if (font->ftlibrary) {
        FT_Done_FreeType(font->ftlibrary);
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

static text_info *_font_get_text_info(so_font *font, font_atlas *atlas, const char *text, int text_len) {
    const unsigned char *p         = (unsigned char *)text;
    const unsigned char *end       = (unsigned char *)text + text_len;
    int                  num_lines = 1;
    int                  num_chars = 0;

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

        num_chars++;
    }

    text_info *info = calloc(1, sizeof(text_info));
    if (!info) {
        return NULL;
    }
    info->lines = calloc(num_lines, sizeof(text_line));
    if (!info->lines) {
        free(info);
        return NULL;
    }
    info->glyphs = calloc(num_chars, sizeof(font_glyph *));
    if (!info->glyphs) {
        free(info);
        free(info->lines);
        return NULL;
    }
    info->num_lines = num_lines;
    info->num_chars = num_chars;
    info->max_width = 0;

    text_line *line;
    uint32_t   codepoint;
    int        line_idx = 0;
    int        char_idx = 0;
    line                = &info->lines[line_idx];
    line->glyphs        = &info->glyphs[char_idx];
    line->line_width    = 0;
    line->num_chars     = 0;

    p = (unsigned char *)text;
    while (p < end) {
        if (*p == '\n') {
            p++;
        break_line:
            if (info->max_width < line->line_width) {
                info->max_width = line->line_width;
            }
            if (line_idx >= info->num_lines) {
                break;
            }
            line_idx++;
            line             = &info->lines[line_idx];
            line->glyphs     = &info->glyphs[char_idx];
            line->line_width = 0;
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
        if (char_idx >= info->num_chars) {
            break;
        }
        line->glyphs[line->num_chars] = _font_get_glyph(font, atlas, codepoint);
        if (line->glyphs[line->num_chars]) {
            line->line_width += line->glyphs[line->num_chars]->advance_x;
            line->num_chars++;
            char_idx++;
        }
    }
    if (info->max_width < line->line_width) {
        info->max_width = line->line_width;
    }

    return info;
}

static void _font_free_text_info(text_info *info) {
    if (!info) return;

    if (info->lines) free(info->lines);
    if (info->glyphs) free(info->glyphs);
    free(info);
}

unsigned char *so_font_create_bitmap(so_font *font, const char *text, int font_size, int linespace, int *out_width, int *out_height) {
    if (!font || !text || font_size <= 0) return NULL;

    font_atlas *atlas = _font_get_atlas(font, font_size);
    if (!atlas) {
        return NULL;
    }

    int        text_len = strlen(text);
    text_info *info     = _font_get_text_info(font, atlas, text, text_len);
    if (!info) {
        return NULL;
    }

    int            line_height = atlas->font_height + linespace;
    int            max_w       = info->max_width;
    int            max_h       = info->num_lines * line_height;
    unsigned char *bitmap      = calloc(1, max_w * max_h);
    if (!bitmap) {
        _font_free_text_info(info);
        return NULL;
    }

    int pen_y = 0;
    for (int l = 0; l < info->num_lines; l++) {
        text_line         *line   = &info->lines[l];
        const font_glyph **glyphs = line->glyphs;
        int                pen_x  = 0;

        for (int x = 0; x < line->num_chars; x++) {
            const font_glyph *glyph  = glyphs[x];
            int               dst_x  = pen_x + glyph->bitmap_left;
            int               dst_y  = pen_y + (atlas->font_ascender - glyph->bitmap_top);
            int               copy_w = glyph->bitmap_width;
            int               copy_h = glyph->bitmap_height;
            if (dst_x + copy_w > max_w) {
                copy_w = max_w - dst_x;
            }
            if (dst_y + copy_h > max_h) {
                copy_h = max_h - dst_y;
            }

            unsigned char *src = glyph->bitmap_buffer;
            unsigned char *dst = bitmap + dst_x + dst_y * max_w;
            for (int y = 0; y < copy_h; y++) {
                memcpy(dst, src, copy_w);
                src += glyph->bitmap_pitch;
                dst += max_w;
            }
            pen_x += glyph->advance_x;
        }

        pen_y += line_height;
    }

    if (out_width) *out_width = max_w;
    if (out_height) *out_height = max_h;

    _font_free_text_info(info);
    return bitmap;
}

size_t so_font_get_font_height(so_font *font, size_t font_size) {
    if (FT_Set_Pixel_Sizes(font->ftface, 0, font_size) != FT_Err_Ok)
        return 0;

    return (font->ftface->size->metrics.height >> 6);
}
