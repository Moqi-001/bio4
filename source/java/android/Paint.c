#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include <so_jni/so_jni.h>

#include "FontMetrics.h"
#include "Paint.h"

struct APaint {
    AFontMetrics mFontMetrics;
    jboolean     mAntiAlias;
    uint32_t     mColor;
    int          mTypeface;
    jsize        mTextSize;
};

APaint *Paint_new() {
    APaint *paint = calloc(1, sizeof(APaint));

    return paint;
}

void Paint_setAntiAlias(APaint *paint, jboolean enabled) {
    paint->mAntiAlias = enabled;
}

void Paint_setColor(APaint *paint, uint32_t color) {
    paint->mColor = color;
}

void Paint_setTypeface(APaint *paint, jint typeface) {
    paint->mTypeface = typeface;
}

void Paint_setTextSize(APaint *paint, jsize size) {
    paint->mTextSize = size;
}

AFontMetrics *Paint_getFontMetrics(APaint *paint) {
    return &paint->mFontMetrics;
}
