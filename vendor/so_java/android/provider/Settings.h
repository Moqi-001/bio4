#ifndef ANDROID_PROVIDER_SETTINGS_H
#define ANDROID_PROVIDER_SETTINGS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <so_java/android/content/ContentResolver.h>
#include <so_java/java/lang/String.h>

void setup_android_provider_Settings(void);

JString Settings_Secure_getString(ContentResolver resolver, JString key);

#ifdef __cplusplus
}
#endif

#endif
