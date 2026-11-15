#ifndef JAVA_LNAG_ARRAY_H
#define JAVA_LNAG_ARRAY_H

#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"

JObject Array_new(JType type, JInt length);
JType   Array_getType(JObject array);
JInt    Array_getLength(JObject array);
JInt    Array_getRegion(JObject array, JInt start, JInt length, void *dst);
JInt    Array_setRegion(JObject array, JInt start, JInt length, const void *src);

#ifdef __cplusplus
}
#endif

#endif