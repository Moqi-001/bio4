#include <string.h>

#include <so_common/so_shared_buffer.h>

#include "so_jni.h"
#include "so_jni_internal.h"

typedef struct so_jstring {
    uint32_t magic;
    jchar   *utf16;
    jsize    utf16_len;
    char    *utf8;
    jsize    utf8_len;
} so_jstring;

#define SO_JSTRING_MAGIC 0x45638218  // "so_jstring".hashCode()

#define SO_JSTRING_CAST(_o) \
    ((so_jstring *)((_o) && ((so_jstring *)(_o))->magic == SO_JSTRING_MAGIC ? (_o) : NULL))

#define SO_JSTRING_FROM_OBJ(_o)                \
    ({                                         \
        void *_inst = so_jobject_get_instance(_o); \
        SO_JSTRING_CAST(_inst);                \
    })

static jsize _utf8_to_utf16_length(const char *utf8, jsize utf8_len) {
    if (!utf8 || utf8_len <= 0) return 0;

    jsize                utf16_len = 0;
    const unsigned char *p         = (const unsigned char *)utf8;
    const unsigned char *end       = p + utf8_len;

    while (p < end) {
        if (*p < 0x80) {  // 1字节 ASCII
            utf16_len++;
            p += 1;
        } else if ((*p & 0xE0) == 0xC0) {  // 2字节 UTF-8
            utf16_len++;
            p += 2;
        } else if ((*p & 0xF0) == 0xE0) {  // 3字节 UTF-8
            utf16_len++;
            p += 3;
        } else if ((*p & 0xF8) == 0xF0) {  // 4字节 UTF-8（代理对）
            utf16_len += 2;
            p += 4;
        } else {  // 非法 UTF-8
            utf16_len++;
            p += 1;
        }
    }

    return utf16_len;
}

static jsize _utf16_to_utf8_length(const jchar *utf16, jsize utf16_len) {
    if (!utf16 || utf16_len <= 0) return 0;

    jsize        utf8_len = 0;
    const jchar *p        = utf16;
    const jchar *end      = p + utf16_len;

    while (p < end) {
        if (*p < 0x80) {  // 1字节 ASCII
            utf8_len += 1;
            p += 1;
        } else if (*p < 0x800) {  // 2字节 UTF-8
            utf8_len += 2;
            p += 1;
        } else if (*p >= 0xD800 && *p <= 0xDBFF) {  // 4字节 UTF-8（代理对）
            utf8_len += 4;
            p += 2;
        } else {  // 3字节 UTF-8
            utf8_len += 3;
            p += 1;
        }
    }

    return utf8_len;
}

static void _utf8_to_utf16(const char *utf8, jsize utf8_len, jchar *utf16_out) {
    if (!utf8 || utf8_len <= 0 || !utf16_out) return;

    const unsigned char *p   = (const unsigned char *)utf8;
    const unsigned char *end = p + utf8_len;

    while (p < end) {
        if (*p < 0x80) {
            *utf16_out++ = *p++;
        } else if ((*p & 0xE0) == 0xC0) {
            if (p + 1 < end) {
                unsigned char c1 = *p++;
                unsigned char c2 = *p++;
                if ((c2 & 0xC0) == 0x80) {
                    *utf16_out++ = ((c1 & 0x1F) << 6) | (c2 & 0x3F);
                } else {
                    *utf16_out++ = 0xFFFD;  // 非法：输出'�'
                }
            } else {
                *utf16_out++ = 0xFFFD;  // 非法：输出'�'
                p            = end;
            }
        } else if ((*p & 0xF0) == 0xE0) {
            if (p + 2 < end) {
                unsigned char c1 = *p++;
                unsigned char c2 = *p++;
                unsigned char c3 = *p++;
                if ((c2 & 0xC0) == 0x80 && (c3 & 0xC0) == 0x80) {
                    *utf16_out++ = ((c1 & 0x0F) << 12) | ((c2 & 0x3F) << 6) | (c3 & 0x3F);
                } else {
                    *utf16_out++ = 0xFFFD;  // 非法：输出'�'
                }
            } else {
                *utf16_out++ = 0xFFFD;  // 非法：输出'�'
                p            = end;
            }
        } else if ((*p & 0xF8) == 0xF0) {
            if (p + 3 < end) {
                unsigned char c1 = *p++;
                unsigned char c2 = *p++;
                unsigned char c3 = *p++;
                unsigned char c4 = *p++;
                if ((c2 & 0xC0) == 0x80 && (c3 & 0xC0) == 0x80 && (c4 & 0xC0) == 0x80) {
                    uint32_t codepoint = ((c1 & 0x07) << 18) | ((c2 & 0x3F) << 12) |
                                         ((c3 & 0x3F) << 6) | (c4 & 0x3F);
                    if (codepoint >= 0x10000 && codepoint <= 0x10FFFF) {
                        *utf16_out++ = 0xD800 + ((codepoint - 0x10000) >> 10);
                        *utf16_out++ = 0xDC00 + ((codepoint - 0x10000) & 0x3FF);
                    } else {
                        *utf16_out++ = 0xFFFD;  // 非法：输出'�'
                    }
                } else {
                    *utf16_out++ = 0xFFFD;  // 非法：输出'�'
                }
            } else {
                *utf16_out++ = 0xFFFD;  // 非法：输出'�'
                p            = end;
            }
        } else {
            *utf16_out++ = 0xFFFD;  // 非法：输出'�'
            p++;
        }
    }
}

static void _utf16_to_utf8(const jchar *utf16, jsize utf16_len, char *utf8_out) {
    if (!utf16 || utf16_len <= 0 || !utf8_out) return;

    const jchar *p   = utf16;
    const jchar *end = p + utf16_len;

    while (p < end) {
        jchar ch = *p++;

        if (ch < 0x80) {
            // 1字节 ASCII
            *utf8_out++ = (char)ch;
        } else if (ch < 0x800) {
            // 2字节 UTF-8: 110xxxxx 10xxxxxx
            *utf8_out++ = (char)(0xC0 | ((ch >> 6) & 0x1F));
            *utf8_out++ = (char)(0x80 | (ch & 0x3F));
        } else if (ch >= 0xD800 && ch <= 0xDBFF) {
            // 高代理项，需要组合成 4 字节 UTF-8
            if (p + 1 < end) {
                jchar high = ch;
                jchar low  = *p++;

                if (low >= 0xDC00 && low <= 0xDFFF) {
                    // 计算 Unicode 码点
                    uint32_t codepoint = 0x10000 + ((high - 0xD800) << 10) + (low - 0xDC00);

                    // 4 字节 UTF-8: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
                    *utf8_out++ = (char)(0xF0 | ((codepoint >> 18) & 0x07));
                    *utf8_out++ = (char)(0x80 | ((codepoint >> 12) & 0x3F));
                    *utf8_out++ = (char)(0x80 | ((codepoint >> 6) & 0x3F));
                    *utf8_out++ = (char)(0x80 | (codepoint & 0x3F));
                } else {
                    // 非法：高代理项后不是低代理项，输出 '�'
                    *utf8_out++ = (char)0xEF;
                    *utf8_out++ = (char)0xBF;
                    *utf8_out++ = (char)0xBD;
                }
            } else {
                // 非法：高代理项后无字符，输出 '�'
                *utf8_out++ = (char)0xEF;
                *utf8_out++ = (char)0xBF;
                *utf8_out++ = (char)0xBD;
            }
        } else if (ch >= 0xDC00 && ch <= 0xDFFF) {
            // 非法：孤立的低代理项，输出 '�'
            *utf8_out++ = (char)0xEF;
            *utf8_out++ = (char)0xBF;
            *utf8_out++ = (char)0xBD;
        } else {
            // 3 字节 UTF-8（普通 BMP 字符）: 11100xxx 10xxxxxx 10xxxxxx
            *utf8_out++ = (char)(0xE0 | ((ch >> 12) & 0x0F));
            *utf8_out++ = (char)(0x80 | ((ch >> 6) & 0x3F));
            *utf8_out++ = (char)(0x80 | (ch & 0x3F));
        }
    }

    *utf8_out = '\0';
}

static jint _ensure_utf16_data(so_jstring *string) {
    if (!string) return JNI_ERR;

    if (string->utf16) return JNI_OK;

    if (!string->utf8 || string->utf8_len <= 0) {
        return JNI_ERR;
    }

    jsize  utf16_len = _utf8_to_utf16_length(string->utf8, string->utf8_len);
    jchar *utf16     = (jchar *)so_shared_buffer_alloc(utf16_len * sizeof(jchar));
    if (!utf16) {
        return JNI_ERR;
    }

    _utf8_to_utf16(string->utf8, string->utf8_len, utf16);
    string->utf16     = utf16;
    string->utf16_len = utf16_len;

    return JNI_OK;
}

static jint _ensure_utf8_data(so_jstring *string) {
    if (!string) return JNI_ERR;

    if (string->utf8) return JNI_OK;

    if (!string->utf16 || string->utf16_len <= 0) {
        return JNI_ERR;
    }

    jsize utf8_len = _utf16_to_utf8_length(string->utf16, string->utf16_len);
    char *utf8     = (char *)so_shared_buffer_alloc(utf8_len + 1);
    if (!utf8) {
        return JNI_ERR;
    }

    _utf16_to_utf8(string->utf16, string->utf16_len, utf8);
    string->utf8     = utf8;
    string->utf8_len = utf8_len;

    return JNI_OK;
}

static jint _jstring_ctor_utf16(so_jstring *string, const jchar *utf16, jsize len) {
    if (!string || !utf16 || len <= 0) return JNI_EINVAL;

    string->magic    = SO_JSTRING_MAGIC;
    string->utf8     = NULL;
    string->utf8_len = 0;
    string->utf16    = so_shared_buffer_alloc_with_data(len * sizeof(jchar), utf16);
    if (!string->utf16) {
        return JNI_ERR;
    }
    string->utf16_len = len;

    return JNI_OK;
}

static jint _jstring_ctor_utf8(so_jstring *string, const char *utf8) {
    if (!string || !utf8) return JNI_EINVAL;

    string->magic     = SO_JSTRING_MAGIC;
    string->utf16     = NULL;
    string->utf16_len = 0;
    jsize len         = strlen(utf8);
    string->utf8      = so_shared_buffer_alloc_with_data(len + 1, utf8);
    if (!string->utf8) {
        return JNI_ERR;
    }
    string->utf8_len = len;

    return JNI_OK;
}

static void _jstring_dtor(so_jstring *string) {
    if (!string) return;

    if (string->utf16) {
        so_shared_buffer_release(string->utf16);
    }
    if (string->utf8) {
        so_shared_buffer_release(string->utf8);
    }
}

so_jstring *so_jstring_new_with_utf16(const jchar *utf16, jsize len) {
    so_jstring *string = (so_jstring *)so_jinstance_new(sizeof(so_jstring), (so_jdtor_t)_jstring_dtor);
    if (!string) {
        return NULL;
    }

    if (_jstring_ctor_utf16(string, utf16, len) != JNI_OK) {
        so_jinstance_release(string);
        return NULL;
    }

    return string;
}

so_jstring *so_jstring_new_with_utf8(const char *utf8) {
    so_jstring *string = (so_jstring *)so_jinstance_new(sizeof(so_jstring), (so_jdtor_t)_jstring_dtor);
    if (!string) {
        return NULL;
    }

    if (_jstring_ctor_utf8(string, utf8) != JNI_OK) {
        so_jinstance_release(string);
        return NULL;
    }

    return string;
}

const jchar *so_jstring_retain_utf16_chars(so_jstring *string) {
    if (!string) return NULL;

    if (_ensure_utf16_data(string) != JNI_OK) return NULL;

    so_shared_buffer_retain(string->utf16);
    return string->utf16;
}

int32_t so_jstring_release_utf16_chars(so_jstring *string, jchar *utf16) {
    if (!string || !utf16) return -1;

    int32_t prev = so_shared_buffer_release(utf16);
    if (prev == 1) {
        if (string->utf16 == utf16) {
            string->utf16 = NULL;
        }
    }
    return prev;
}

jint so_jstring_read_utf16_chars(so_jstring *string, jsize start, jsize length, jchar *dst) {
    if (!string || !dst || start < 0 || length <= 0) return JNI_EINVAL;

    if (_ensure_utf16_data(string) != JNI_OK) return JNI_ERR;

    if (!string->utf16 || start + length > string->utf16_len) return JNI_ERR;

    if (string->utf16 != dst) {
        memcpy(dst, string->utf16 + start, length * sizeof(jchar));
    }

    return JNI_OK;
}

const jchar *so_jstring_get_utf16_chars(so_jstring *string) {
    if (!string) return NULL;

    if (_ensure_utf16_data(string) != JNI_OK) return NULL;

    return string->utf16;
}

jsize so_jstring_get_utf16_length(so_jstring *string) {
    if (!string) return 0;

    if (_ensure_utf16_data(string) != JNI_OK) return 0;

    return string->utf16_len;
}

const char *so_jstring_retain_utf8_chars(so_jstring *string) {
    if (!string) return NULL;

    if (_ensure_utf8_data(string) != JNI_OK) return NULL;

    so_shared_buffer_retain(string->utf8);
    return string->utf8;
}

int32_t so_jstring_release_utf8_chars(so_jstring *string, char *utf8) {
    if (!string || !utf8) return -1;

    int32_t prev = so_shared_buffer_release(utf8);
    if (prev == 1) {
        if (string->utf8 == utf8) {
            string->utf8 = NULL;
        }
    }

    return prev;
}

jint so_jstring_read_utf8_chars(so_jstring *string, jsize start, jsize length, char *dst) {
    if (!string || !dst || start < 0 || length <= 0) return JNI_EINVAL;

    if (_ensure_utf8_data(string) != JNI_OK) return JNI_ERR;

    if (!string->utf8 || start + length > string->utf8_len) return JNI_ERR;

    if (string->utf8 != dst) {
        memcpy(dst, string->utf8 + start, length);
    }

    return JNI_OK;
}

const char *so_jstring_get_utf8_chars(so_jstring *string) {
    if (!string) return NULL;

    if (_ensure_utf8_data(string) != JNI_OK) return NULL;

    return string->utf8;
}

jsize so_jstring_get_utf8_length(so_jstring *string) {
    if (!string) return 0;

    if (_ensure_utf8_data(string) != JNI_OK) return 0;

    return string->utf8_len;
}

so_jstring *so_jstring_from_object(jstring string) {
    return SO_JSTRING_FROM_OBJ(string);
}
