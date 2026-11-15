#ifndef ANDROID_CONTENT_CONTENTRESOLVER_H
#define ANDROID_CONTENT_CONTENTRESOLVER_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _ContentResolver *ContentResolver;

void setup_android_content_ContentResolver(void);

ContentResolver ContentResolver_new();

#ifdef __cplusplus
}
#endif

#endif
