#include <string.h>

#include "Array.h"
#include "Object.h"

typedef struct JavaArrayHeader {
    uint32_t magic;
    JInt     type;
    JInt     length;
    JInt     elem_size;
    void    *elems;
} JavaArrayHeader;

#define JAVA_ARRAY_MAGIC    0x4388FD37  // "JObject".hashCode()
#define JAVA_ARRAY_CAST(_o) JAVA_BASE_CAST(JavaArrayHeader *, _o, JAVA_ARRAY_MAGIC)

#define headerFromArray(_elems) (JavaArrayHeader *)(((char *)(_elems) + Object_getSize(_elems)) - sizeof(JavaArrayHeader))

#define ALIGN_UP(x, align) (((x) + (align) - 1) & ~((align) - 1))

static JInt _type_size(JType type) {
    switch (type) {
        case JTYPE_BOOLEAN:
            return sizeof(JBoolean);
        case JTYPE_BYTE:
            return sizeof(JByte);
        case JTYPE_CHAR:
            return sizeof(JChar);
        case JTYPE_SHORT:
            return sizeof(JShort);
        case JTYPE_INT:
            return sizeof(JInt);
        case JTYPE_LONG:
            return sizeof(JLong);
        case JTYPE_FLOAT:
            return sizeof(JFloat);
        case JTYPE_DOUBLE:
            return sizeof(JDouble);
        case JTYPE_OBJECT:
            return sizeof(JObject);
        default:
            return 0;
    }
}

static void _array_dtor(JObject arr) {
    JavaArrayHeader *header = headerFromArray(arr);
    if (header->type == JTYPE_OBJECT) {
        JObject *elems = (JObject *)header->elems;
        for (int i = 0; i < header->length; i++) {
            if (elems[i]) {
                Object_release(elems[i]);
            }
        }
    }
}

JObject Array_new(JType type, JInt length) {
    JInt elem_size = _type_size(type);
    if (length <= 0 || elem_size <= 0) {
        return NULL;
    }

    JInt elems_size = elem_size * length;
    elems_size      = ALIGN_UP(elems_size, sizeof(uintptr_t));
    JObject array   = Object_newWithoutClass(elems_size + sizeof(JavaArrayHeader), (JDtor)_array_dtor);
    if (!array) {
        return NULL;
    }

    if (type == JTYPE_OBJECT) {
        memset(array, 0, elems_size);
    }

    JavaArrayHeader *header = headerFromArray(array);
    header->magic           = JAVA_ARRAY_MAGIC;
    header->type            = type;
    header->length          = length;
    header->elem_size       = elem_size;
    header->elems           = array;

    return array;
}

JType Array_getType(JObject array) {
    if (!array) {
        return JTYPE_UNKNOWN;
    }

    JavaArrayHeader *header = JAVA_ARRAY_CAST(headerFromArray(array));
    if (!header) {
        return JTYPE_UNKNOWN;
    }
    return header->type;
}

JInt Array_getLength(JObject array) {
    if (!array) {
        return 0;
    }

    JavaArrayHeader *header = JAVA_ARRAY_CAST(headerFromArray(array));
    if (!header) {
        return 0;
    }
    return header->length;
}

JInt Array_getRegion(JObject array, JInt start, JInt length, void *dst) {
    if (!array || !dst || start < 0 || length <= 0) {
        return JAVA_ERR;
    }

    JavaArrayHeader *header = JAVA_ARRAY_CAST(headerFromArray(array));
    if (!header) {
        return JAVA_ERR;
    }
    if (start + length > header->length) {
        return JAVA_ERR;
    }

    void *src = (char *)array + start * header->elem_size;
    memcpy(dst, src, length * header->elem_size);

    return JAVA_OK;
}

JInt Array_setRegion(JObject array, JInt start, JInt length, const void *src) {
    if (!array || !src || start < 0 || length <= 0) {
        return JAVA_ERR;
    }

    JavaArrayHeader *header = JAVA_ARRAY_CAST(headerFromArray(array));
    if (!header) {
        return JAVA_ERR;
    }
    if (start + length > header->length) {
        return JAVA_ERR;
    }

    void *dst = (char *)array + start * header->elem_size;
    if (src == dst) {
        return JAVA_OK;  // 无需自拷贝
    }
    memcpy(dst, src, length * header->elem_size);

    return JAVA_OK;
}
