#ifndef ANDROID_VIEW_SURFACE_H
#define ANDROID_VIEW_SURFACE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _Surface *Surface;

void setup_android_view_Surface(void);

Surface Surface_new();

#ifdef __cplusplus
}
#endif

#endif