#ifndef JAVA_LANG_STRING_H
#define JAVA_LANG_STRING_H

#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"

JString      String_new(const JChar *utf16, JInt len);
const JChar *String_getChars(JString string);
JInt         String_getLength(JString string);
JInt         String_getRegion(JString string, JInt start, JInt length, JChar *dst);

JString     String_newUTF(const char *bytes);
const char *String_getUTFChars(JString string);
JInt        String_getUTFLength(JString string);
JInt        String_getUTFRegion(JString string, JInt start, JInt length, char *dst);

JString String_trim(JString string);

#ifdef __cplusplus
}
#endif

#endif