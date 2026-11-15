#ifndef JAVA_UTIL_LOCALE_H
#define JAVA_UTIL_LOCALE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <so_java/java/lang/String.h>

typedef struct _Locale *Locale;

void setup_java_util_Locale(void);

Locale Locale_new();
Locale Locale_getDefault();

#ifdef __cplusplus
}
#endif

#endif
