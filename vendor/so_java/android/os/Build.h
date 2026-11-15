#ifndef ANDROID_OS_BUILD_H
#define ANDROID_OS_BUILD_H

#ifdef __cplusplus
extern "C" {
#endif

#include <so_java/java/lang/types.h>

typedef struct _Build *Build;

extern JInt android_os_Build_VERSION_SDK_INT;

void setup_android_os_Build(void);

#ifdef __cplusplus
}
#endif

#endif