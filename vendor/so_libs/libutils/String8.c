#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include <so_common/so_logger.h>

#include "SharedBuffer.h"
#include "String8.h"

struct AString8 {
    char *string;
};

#define bufferFromString(string) (ASharedBuffer_bufferFromData(string))
#define lengthFromString(string) (ASharedBuffer_sizeFromData(string) - 1)
#define stringFromBuffer(buffer) (ASharedBuffer_data(buffer))
#define lengthFromBuffer(buffer) (ASharedBuffer_size(buffer) - 1)

static int _real_ctor(AString8 *thiz, const char *cstr, size_t len) {
    ASharedBuffer *buffer;

    if (!cstr || len == 0) {  // 创建空字符串
        buffer = ASharedBuffer_alloc(1);
        if (!buffer)
            return -1;
        thiz->string      = stringFromBuffer(buffer);
        len               = 0;
        thiz->string[len] = '\0';
    } else {  // 创建并复制字符串
        buffer = ASharedBuffer_alloc(len + 1);
        if (!buffer)
            return -1;
        thiz->string = stringFromBuffer(buffer);
        memcpy(thiz->string, cstr, len);
        thiz->string[len] = '\0';
    }

    return 0;
}

static int _real_setTo(AString8 *thiz, const char *cstr, size_t len) {
    if (!thiz->string) {
        return _real_ctor(thiz, cstr, len);
    }

    ASharedBuffer *buffer = bufferFromString(thiz->string);

    if (!cstr || len == 0) {  // 写入空字符串
        buffer = ASharedBuffer_editResize(buffer, 1);
        if (!buffer)
            return -1;
        thiz->string      = stringFromBuffer(buffer);
        len               = 0;
        thiz->string[len] = '\0';
    } else {  // 复制字符串
        buffer = ASharedBuffer_editResize(buffer, len + 1);
        if (!buffer)
            return -1;
        thiz->string = stringFromBuffer(buffer);
        memcpy(thiz->string, cstr, len);
        thiz->string[len] = '\0';
    }

    return 0;
}

static int _real_append(AString8 *thiz, const char *cstr, size_t len) {
    if (!thiz->string) {
        return _real_ctor(thiz, cstr, len);
    }

    if (!cstr || len == 0) {
        return 0;
    }

    ASharedBuffer *buffer   = bufferFromString(thiz->string);
    size_t         thiz_len = lengthFromBuffer(buffer);
    buffer                  = ASharedBuffer_editResize(buffer, thiz_len + len + 1);
    if (!buffer)
        return -1;

    thiz->string = stringFromBuffer(buffer);
    memcpy(thiz->string + thiz_len, cstr, len);
    thiz_len += len;
    thiz->string[thiz_len] = '\0';

    return 0;
}

static int _real_appendPath(AString8 *thiz, const char *cstr, size_t len) {
    if (!thiz->string) {
        return _real_ctor(thiz, cstr, len);
    }

    if (!cstr || len == 0) {
        return 0;
    }

    ASharedBuffer *buffer;
    size_t         thiz_len;

    if (cstr[0] != '/') {
        buffer   = bufferFromString(thiz->string);
        thiz_len = lengthFromBuffer(buffer);
        if (thiz->string[thiz_len] != '/') {
            buffer = ASharedBuffer_editResize(buffer, thiz_len + len + 2);  // 多分配一个字节给'/'
            if (!buffer)
                return -1;
            thiz->string             = stringFromBuffer(buffer);
            thiz->string[thiz_len++] = '/';
        } else {
            buffer = ASharedBuffer_editResize(buffer, thiz_len + len + 1);
            if (!buffer)
                return -1;
            thiz->string = stringFromBuffer(buffer);
        }

        memcpy(thiz->string + thiz_len, cstr, len);
        thiz_len += len;
        thiz->string[thiz_len] = '\0';
    } else {
        buffer = ASharedBuffer_editResize(buffer, len + 1);
        if (!buffer)
            return -1;
        thiz->string = stringFromBuffer(buffer);
        memcpy(thiz->string, cstr, len);
        thiz_len               = len;
        thiz->string[thiz_len] = '\0';
    }

    return 0;
}

void AString8_ctor(AString8 *thiz) {
    _real_ctor(thiz, NULL, 0);
    so_logd("[NDK][String8] ctor(%p): '%s'", thiz, thiz->string);
}

void AString8_ctorWithCstr(AString8 *thiz, const char *cstr) {
    size_t len = cstr ? strlen(cstr) : 0;
    _real_ctor(thiz, cstr, len);
    so_logd("[NDK][String8] ctorWithCstr(%p,%p): '%s'", thiz, cstr, thiz->string);
}

void AString8_ctorWithOther(AString8 *thiz, const AString8 *other) {
    if (other && other->string) {
        ASharedBuffer_acquire(bufferFromString(other->string));
        thiz->string = other->string;
    } else {
        _real_ctor(thiz, NULL, 0);
    }
    so_logd("[NDK][String8] ctorWithOther(%p,%p): '%s'", thiz, other, thiz->string);
}

void AString8_dtor(AString8 *thiz) {
    so_logd("[NDK][String8] dtor(%p): '%s'", thiz, thiz->string);

    if (thiz->string) {
        ASharedBuffer_release(bufferFromString(thiz->string), 0);
        thiz->string = NULL;
    }
}

int AString8_setTo(AString8 *thiz, const AString8 *other) {
    if (other) {
        ASharedBuffer_release(bufferFromString(thiz->string), 0);
        ASharedBuffer_acquire(bufferFromString(other->string));
        thiz->string = other->string;
    } else {
        if (_real_setTo(thiz, NULL, 0) < 0) {
            so_loge("[NDK][String8] setTo(%p,%p): error!", thiz, other);
            return -1;
        }
    }

    so_logd("[NDK][String8] setTo(%p,%p): '%s'", thiz, other, thiz->string);
    return 0;
}

int AString8_setToCstr(AString8 *thiz, const char *cstr) {
    size_t len = cstr ? strlen(cstr) : 0;

    if (_real_setTo(thiz, cstr, len) < 0) {
        so_loge("[NDK][String8] setToCstr(%p,%p): error!", thiz, cstr);
        return -1;
    }

    so_logd("[NDK][String8] setToCstr(%p,%p): '%s'", thiz, cstr, thiz->string);
    return 0;
}

int AString8_append(AString8 *thiz, const AString8 *other) {
    const char *cstr = other ? other->string : NULL;
    size_t      len  = other ? lengthFromString(other->string) : 0;

    if (_real_append(thiz, cstr, len) < 0) {
        so_loge("[NDK][String8] append(%p,%p): error!", thiz, other);
        return -1;
    }

    so_logd("[NDK][String8] append(%p,%p): '%s'", thiz, other, thiz->string);
    return 0;
}

int AString8_appendCstr(AString8 *thiz, const char *cstr) {
    size_t len = cstr ? strlen(cstr) : 0;

    if (_real_append(thiz, cstr, len) < 0) {
        so_loge("[NDK][String8] appendCstr(%p,%p): error!", thiz, cstr);
        return -1;
    }

    so_logd("[NDK][String8] appendCstr(%p,%p): '%s'", thiz, cstr, thiz->string);
    return 0;
}

int AString8_appendPath(AString8 *thiz, const AString8 *other) {
    const char *cstr = other ? other->string : NULL;
    size_t      len  = other ? lengthFromString(other->string) : 0;

    if (_real_appendPath(thiz, cstr, len) < 0) {
        so_loge("[NDK][String8] appendPath(%p,%p): error!", thiz, other);
        return -1;
    }

    so_logd("[NDK][String8] appendPath(%p,%p): '%s'", thiz, other, thiz->string);
    return 0;
}

int AString8_appendPathCstr(AString8 *thiz, const char *cstr) {
    size_t len = cstr ? strlen(cstr) : 0;

    if (_real_appendPath(thiz, cstr, len) < 0) {
        so_loge("[NDK][String8] appendPathCstr(%p,%p): error!", thiz, cstr);
        return -1;
    }

    so_logd("[NDK][String8] appendPathCstr(%p,%p): '%s'", thiz, cstr, thiz->string);
    return 0;
}

int AString8_getPathDir(AString8 *thiz, const AString8 *other) {
    int result;

    if (!other || !other->string) {  // 创建空字符串
        result = _real_ctor(thiz, NULL, 0);
    } else {  // 创建并获取父路径
        const char *cstr = other->string;
        const char *cp   = strrchr(cstr, '/');
        if (cp == NULL)
            cp = cstr;

        size_t len = cp - cstr;
        result     = _real_ctor(thiz, cstr, len);
    }

    if (result < 0) {
        so_loge("[NDK][String8] getPathDir(%p,%p): error!", thiz, other);
        return -1;
    }

    so_logd("[NDK][String8] getPathDir(%p,%p): '%s'", thiz, other, thiz->string);
    return result;
}

int AString8_walkPath(AString8 *thiz, const AString8 *other, AString8 *out_remains) {
    int result;

    if (!other || !other->string)  // 创建空字符串
    {
        result = _real_ctor(thiz, NULL, 0);
        if (out_remains) {
            _real_ctor(out_remains, NULL, 0);
        }
    } else {
        const char *cstr = other->string;
        const char *tail = cstr + lengthFromString(other->string);
        const char *buf  = cstr;
        const char *cp   = strchr(buf, '/');
        if (cp == buf) {
            // don't include a leading '/'.
            buf = buf + 1;
            cp  = strchr(buf, '/');
        }
        if (cp == NULL)
            cp = tail;

        size_t len = cp - buf;

        result = _real_ctor(thiz, buf, len);
        if (out_remains) {
            buf = (cp < tail) ? (cp + 1) : tail;
            len = tail - buf;
            _real_ctor(out_remains, buf, len);
        }
    }

    if (result < 0) {
        so_loge("[NDK][String8] walkPath(%p,%p,%p): error!", thiz, other, out_remains);
        return -1;
    }

    so_logd("[NDK][String8] walkPath(%p,%p,%p): '%s','%s'", thiz, other, out_remains, thiz->string, out_remains ? out_remains->string : NULL);
    return result;
}

int AString8_empty(AString8 *thiz) {
    return _real_ctor(thiz, NULL, 0);
}

const char *AString8_string(AString8 *thiz) {
    return thiz->string;
}

size_t AString8_length(AString8 *thiz) {
    return lengthFromString(thiz->string);
}

int AString8_isEmpty(AString8 *thiz) {
    return (lengthFromString(thiz->string) == 0);
}
