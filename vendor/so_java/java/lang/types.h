#ifndef JAVA_LANG_TYPES_H
#define JAVA_LANG_TYPES_H

#include <stdint.h>

typedef enum JType {
    JTYPE_UNKNOWN = 0,
    JTYPE_BOOLEAN = 1,  // Z
    JTYPE_BYTE    = 2,  // B
    JTYPE_CHAR    = 3,  // C
    JTYPE_SHORT   = 4,  // S
    JTYPE_INT     = 5,  // I
    JTYPE_LONG    = 6,  // J
    JTYPE_FLOAT   = 7,  // F
    JTYPE_DOUBLE  = 8,  // D
    JTYPE_OBJECT  = 9,  // L
} JType;

typedef uint8_t  JBoolean; /* unsigned 8 bits */
typedef int8_t   JByte;    /* signed 8 bits */
typedef uint16_t JChar;    /* unsigned 16 bits */
typedef int16_t  JShort;   /* signed 16 bits */
typedef int32_t  JInt;     /* signed 32 bits */
typedef int64_t  JLong;    /* signed 64 bits */
typedef float    JFloat;   /* 32-bit IEEE 754 */
typedef double   JDouble;  /* 64-bit IEEE 754 */

typedef void   *JObject;
typedef JObject JClass;
typedef JObject JString;

typedef JObject JMethod;

typedef void (*JDtor)(JObject obj);

#ifdef DEBUG_SOLOADER
#define JAVA_BASE_CAST(_type, _obj, _magic) \
    ((_type)(((_obj) && (*(uint32_t *)(_obj) == _magic)) ? (_obj) : NULL))
#else
#define JAVA_BASE_CAST(_type, _obj, _magic) ((_type)(_obj))
#endif

#define JAVA_TRUE  1
#define JAVA_FALSE 0

#define JAVA_OK  (0)  /* no error */
#define JAVA_ERR (-1) /* generic error */

#endif