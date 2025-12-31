#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "reimpl/android/SharedBuffer.h"
#include "reimpl/android/String8.h"
#include "utils/logger.h"

struct String8 {
    char *string;
};

#define getSharedBuffer(str8) (android_SharedBuffer_bufferFromData(str8->string))
#define getString(sb)         (android_SharedBuffer_data(sb))
#define getLength(sb)         (android_SharedBuffer_size(sb) - 1)

static int real_init(String8 *str8, const char *s, size_t len) {
    if (!str8)
        return -1;

    SharedBuffer *sb;
    size_t        str8_len;

    if (!s || len == 0)  // 创建并写入空字符串
    {
        sb = android_SharedBuffer_alloc(1);
        if (!sb)
            return -1;
        str8->string           = getString(sb);
        str8_len               = 0;
        str8->string[str8_len] = '\0';
    } else  // 创建并复制字符串
    {
        sb = android_SharedBuffer_alloc(len + 1);
        if (!sb)
            return -1;
        str8->string = getString(sb);
        memcpy(str8->string, s, len);
        str8_len               = len;
        str8->string[str8_len] = '\0';
    }

    return 0;
}

static int real_setTo(String8 *str8, const char *s, size_t len) {
    if (!str8 || !str8->string)
        return -1;

    SharedBuffer *sb       = getSharedBuffer(str8);
    size_t        str8_len = getLength(sb);

    if (!s || len == 0)  // 写入空字符串
    {
        sb = android_SharedBuffer_editResize(sb, 1);
        if (!sb)
            return -1;
        str8->string           = getString(sb);
        str8_len               = 0;
        str8->string[str8_len] = '\0';
    } else  // 复制字符串
    {
        sb = android_SharedBuffer_editResize(sb, len + 1);
        if (!sb)
            return -1;
        str8->string = getString(sb);
        memcpy(str8->string, s, len);
        str8_len               = len;
        str8->string[str8_len] = '\0';
    }

    return 0;
}

static int real_append(String8 *str8, const char *s, size_t len) {
    if (!str8 || !str8->string)
        return -1;

    if (!s || len == 0)
        return 0;

    SharedBuffer *sb       = getSharedBuffer(str8);
    size_t        str8_len = getLength(sb);
    sb                     = android_SharedBuffer_editResize(sb, str8_len + len + 1);
    if (!sb)
        return -1;

    str8->string = getString(sb);
    memcpy(str8->string + str8_len, s, len);
    str8_len += len;
    str8->string[str8_len] = '\0';

    return 0;
}

static int real_appendPath(String8 *str8, const char *s, size_t len) {
    if (!str8 || !str8->string)
        return -1;

    if (!s || len == 0)
        return 0;

    SharedBuffer *sb;
    size_t        str8_len;

    if (s[0] != '/') {
        sb       = getSharedBuffer(str8);
        str8_len = getLength(sb);
        if (str8->string[str8_len] != '/') {
            sb = android_SharedBuffer_editResize(sb, str8_len + len + 2);  // 多分配一个字节给'/'
            if (!sb)
                return -1;
            str8->string             = getString(sb);
            str8->string[str8_len++] = '/';
        } else {
            sb = android_SharedBuffer_editResize(sb, str8_len + len + 1);
            if (!sb)
                return -1;
            str8->string = getString(sb);
        }

        memcpy(str8->string + str8_len, s, len);
        str8_len += len;
        str8->string[str8_len] = '\0';
    } else {
        sb = android_SharedBuffer_editResize(sb, len + 1);
        if (!sb)
            return -1;
        str8->string = getString(sb);
        memcpy(str8->string, s, len);
        str8_len               = len;
        str8->string[str8_len] = '\0';
    }

    return 0;
}

int android_String8_init(String8 *str8) {
    l_debug("android_String8_init(%p)", str8);

    int res = real_init(str8, NULL, 0);

    l_debug("android_String8_init over: \"%s\"", str8 ? str8->string : NULL);
    return res;
}

int android_String8_initFromCstring(String8 *str8, const char *s) {
    l_debug("android_String8_initFromCstring(%p,\"%s\")", str8, s);

    size_t len = s ? strlen(s) : 0;
    int    res = real_init(str8, s, len);

    l_debug("android_String8_initFromCstring over: \"%s\"", str8 ? str8->string : NULL);
    return res;
}

int android_String8_initFromString8(String8 *str8, const String8 *src) {
    l_debug("android_String8_initFromString8(%p,%p): \"%s\"", str8, src, src ? src->string : NULL);

    int res = -1;

    if (str8) {
        if (src && src->string) {
            android_SharedBuffer_acquire(getSharedBuffer(src));
            str8->string = src->string;
            res          = 0;
        } else {
            str8->string = NULL;
        }
    }

    l_debug("android_String8_initFromString8 over: \"%s\"", str8 ? str8->string : NULL);
    return res;
}

void android_String8_term(String8 *str8) {
    l_debug("android_String8_term(%p): \"%s\"", str8, str8 ? str8->string : NULL);

    if (str8 && str8->string) {
        android_SharedBuffer_release(getSharedBuffer(str8), 0);
        str8->string = NULL;
    }

    l_debug("android_String8_term over");
}

int android_String8_setTo(String8 *str8, const String8 *src) {
    l_debug("android_String8_setTo(%p,%p): \"%s\"", str8, src, src ? src->string : NULL);

    int res = -1;

    if (str8) {
        if (str8->string)
            android_SharedBuffer_release(getSharedBuffer(str8), 0);

        if (src && src->string) {
            android_SharedBuffer_acquire(getSharedBuffer(src));
            str8->string = src->string;
            res          = 0;
        } else {
            str8->string = NULL;
        }
    }

    l_debug("android_String8_setTo over: \"%s\"", str8 ? str8->string : NULL);
    return res;
}

int android_String8_setToCstring(String8 *str8, const char *s) {
    l_debug("android_String8_setToCstring(%p,\"%s\")", str8, s);

    size_t len = s ? strlen(s) : 0;
    int    res = real_setTo(str8, s, len);

    l_debug("android_String8_setToCstring over: \"%s\"", str8 ? str8->string : NULL);
    return res;
}

int android_String8_append(String8 *str8, const String8 *src) {
    l_debug("android_String8_append(%p,%p): \"%s\",\"%s\"", str8, src, str8 ? str8->string : NULL, src ? src->string : NULL);

    const char *s   = src ? src->string : NULL;
    size_t      len = src ? getLength(getSharedBuffer(src)) : 0;
    int         res = real_append(str8, s, len);

    l_debug("android_String8_append over: \"%s\"", str8 ? str8->string : NULL);
    return res;
}

int android_String8_appendCstring(String8 *str8, const char *s) {
    l_debug("android_String8_appendCstring(%p): \"%s\",\"%s\"", str8, str8 ? str8->string : NULL, s);

    size_t len = s ? strlen(s) : 0;
    int    res = real_append(str8, s, len);

    l_debug("android_String8_appendCstring over: \"%s\"", str8 ? str8->string : NULL);
    return res;
}

int android_String8_appendPath(String8 *str8, const String8 *src) {
    l_debug("android_String8_appendPath(%p,%p): \"%s\",\"%s\"", str8, src, str8 ? str8->string : NULL, src ? src->string : NULL);

    const char *s   = src ? src->string : NULL;
    size_t      len = src ? getLength(getSharedBuffer(src)) : 0;
    int         res = real_appendPath(str8, s, len);

    l_debug("android_String8_appendPath over: \"%s\"", str8 ? str8->string : NULL);
    return res;
}

int android_String8_appendPathCstring(String8 *str8, const char *s) {
    l_debug("android_String8_appendPathCstring(%p): \"%s\",\"%s\"", str8, str8 ? str8->string : NULL, s);

    size_t len = s ? strlen(s) : 0;
    int    res = real_appendPath(str8, s, len);

    l_debug("android_String8_appendPathCstring over: \"%s\"", str8 ? str8->string : NULL);
    return res;
}

int android_String8_getPathDir(String8 *str8, const String8 *src) {
    l_debug("android_String8_getPathDir(%p,%p): \"%s\"", str8, src, src ? src->string : NULL);

    int res;

    if (!src || !src->string)  // 创建空字符串
    {
        res = real_init(str8, NULL, 0);
    } else  // 创建并获取父路径
    {
        const char *s  = src->string;
        const char *cp = strrchr(s, '/');
        if (cp == NULL)
            cp = s;

        size_t len = cp - s;
        res        = real_init(str8, s, len);
    }

    l_debug("android_String8_getPathDir over: \"%s\"", str8 ? str8->string : NULL);
    return res;
}

int android_String8_walkPath(String8 *str8, const String8 *src, String8 *out_remains) {
    l_debug("android_String8_walkPath(%p,%p,%p): \"%s\"", str8, src, out_remains, src ? src->string : NULL);

    int res;

    if (!src || !src->string)  // 创建空字符串
    {
        res = real_init(str8, NULL, 0);
        if (out_remains && out_remains->string)
            real_setTo(out_remains, NULL, 0);
    } else {
        const char *s    = src->string;
        const char *tail = s + getLength(getSharedBuffer(src));
        const char *buf  = s;
        const char *cp   = strchr(buf, '/');
        if (cp == buf) {
            // don't include a leading '/'.
            buf = buf + 1;
            cp  = strchr(buf, '/');
        }
        if (cp == NULL)
            cp = tail;

        size_t len = cp - buf;

        res = real_init(str8, buf, len);
        if (out_remains && out_remains->string) {
            buf = (cp < tail) ? (cp + 1) : tail;
            len = tail - buf;
            real_setTo(out_remains, buf, len);
        }
    }

    l_debug("android_String8_walkPath over: \"%s\",\"%s\"", str8 ? str8->string : NULL, out_remains ? out_remains->string : NULL);
    return res;
}

int android_String8_empty(String8 *str8) {
    return real_setTo(str8, NULL, 0);
}

const char *android_String8_string(String8 *str8) {
    return str8 ? str8->string : NULL;
}

size_t android_String8_length(String8 *str8) {
    return (str8 && str8->string) ? getLength(getSharedBuffer(str8)) : 0;
}

int android_String8_isEmpty(String8 *str8) {
    return (str8 && str8->string) ? (getLength(getSharedBuffer(str8)) == 0) : 0;
}
