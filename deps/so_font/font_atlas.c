#include <stdlib.h>
#include <string.h>

#include <freetype2/ft2build.h>
#include FT_FREETYPE_H

#include "font_atlas.h"
#include "font_htab.h"

font_atlas *font_atlas_new(int font_size, int atlas_w, int atlas_h) {
    font_atlas *atlas = calloc(1, sizeof(font_atlas));
    if (!atlas) {
        return NULL;
    }

    atlas->page_width  = atlas_w;
    atlas->page_height = atlas_h;
    atlas->pen_x       = 0;
    atlas->pen_y       = 0;
    atlas->y_space     = 0;

    atlas->pages = calloc(1, sizeof(font_atlas_page));
    if (!atlas->pages) {
        free(atlas);
        return NULL;
    }

    atlas->pages->data = malloc(atlas_w * atlas_h);
    if (!atlas->pages->data) {
        free(atlas->pages);
        free(atlas);
        return NULL;
    }
    atlas->pages->width  = atlas_w;
    atlas->pages->height = atlas_h;

    atlas->htab = font_htab_new(256);
    if (!atlas->htab) {
        free(atlas->pages->data);
        free(atlas->pages);
        free(atlas);
        return NULL;
    }

    return atlas;
}

void font_atlas_free(font_atlas *atlas) {
    if (!atlas) return;

    font_atlas_page *page = atlas->pages;
    while (page) {
        font_atlas_page *prev = page->prev;
        if (page->data) {
            free(page->data);
        }
        free(page);
        page = prev;
    }
    if (atlas->htab) {
        font_htab_free(atlas->htab);
    }
    free(atlas);
}

const font_glyph *font_atlas_insert(font_atlas *atlas, uint32_t codepoint, const FT_GlyphSlot ft_glyph) {
    if (!atlas || !ft_glyph) return NULL;

    font_glyph *new_glyph = calloc(1, sizeof(font_glyph));
    if (!new_glyph)
        return NULL;

    font_atlas_page *page = atlas->pages;

    if (atlas->pen_x + ft_glyph->bitmap.width > page->width) {  // 行已满
        atlas->pen_x = 0;
        atlas->pen_y += atlas->y_space;
        atlas->y_space = ft_glyph->bitmap.rows;
    }
    if (atlas->pen_y + ft_glyph->bitmap.rows > page->height) {  // 页已满
        page = calloc(1, sizeof(font_atlas_page));
        if (!page) {
            free(new_glyph);
            return NULL;
        }

        page->data = malloc(atlas->page_width * atlas->page_height);
        if (!page->data) {
            free(new_glyph);
            free(page);
            return NULL;
        }

        atlas->pen_x   = 0;
        atlas->pen_y   = 0;
        atlas->y_space = ft_glyph->bitmap.rows;
        page->prev     = atlas->pages;
        atlas->pages   = page;
    }

    new_glyph->bitmap_buffer = page->data + atlas->pen_x + atlas->pen_y * page->width;
    new_glyph->bitmap_pitch  = page->width;
    new_glyph->bitmap_width  = ft_glyph->bitmap.width;
    new_glyph->bitmap_height = ft_glyph->bitmap.rows;
    new_glyph->bitmap_top    = ft_glyph->bitmap_top;
    new_glyph->bitmap_left   = ft_glyph->bitmap_left;
    new_glyph->advance_x     = ft_glyph->advance.x >> 6;
    new_glyph->advance_y     = ft_glyph->advance.y >> 6;

    unsigned char *src = ft_glyph->bitmap.buffer;
    unsigned char *dst = new_glyph->bitmap_buffer;

    if (ft_glyph->bitmap.buffer) {
        for (int y = 0; y < ft_glyph->bitmap.rows; y++) {
            for (int x = 0; x < ft_glyph->bitmap.width; x++) {
                if (ft_glyph->bitmap.pixel_mode == FT_PIXEL_MODE_MONO) {
                    int idx  = x >> 3;           // (x / 8)
                    int mask = 0x80 >> (x & 7);  // (0x80 >> (x % 8))
                    dst[x]   = (src[idx] & mask) ? 0xFF : 0;
                } else if (ft_glyph->bitmap.pixel_mode == FT_PIXEL_MODE_GRAY) {
                    dst[x] = src[x];
                }
            }
            src += ft_glyph->bitmap.pitch;
            dst += new_glyph->bitmap_pitch;
        }
    } else {  // 空格等一些特殊字符没有像素
        for (int y = 0; y < ft_glyph->bitmap.rows; y++) {
            memset(dst, 0, ft_glyph->bitmap.width);
            dst += new_glyph->bitmap_pitch;
        }
    }

    if (!font_htab_insert(atlas->htab, codepoint, new_glyph)) {
        free(new_glyph);
        free(page);
        return NULL;
    }

    atlas->pen_x += new_glyph->bitmap_width;
    if (atlas->y_space < new_glyph->bitmap_height)
        atlas->y_space = new_glyph->bitmap_height;

    return new_glyph;
}

const font_glyph *font_atlas_find(font_atlas *atlas, uint32_t codepoint) {
    return (const font_glyph *)font_htab_find(atlas->htab, codepoint);
}
