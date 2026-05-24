#ifndef ANDROID_PAINT_H
#define ANDROID_PAINT_H

#include <stdint.h>

#include <so_jni/so_jni.h>

#include "FontMetrics.h"

typedef struct APaint APaint;

APaint       *Paint_new();
void          Paint_setAntiAlias(APaint *paint, jboolean enabled);
void          Paint_setColor(APaint *paint, uint32_t color);
void          Paint_setTypeface(APaint *paint, jint typeface);
void          Paint_setTextSize(APaint *paint, jsize size);
AFontMetrics *Paint_getFontMetrics(APaint *paint);

#endif