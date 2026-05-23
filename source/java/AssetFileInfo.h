#ifndef ASSET_FILE_INFO_H
#define ASSET_FILE_INFO_H

#include <stdbool.h>

#include <so_jni/so_jni.h>

extern so_jclass_meta AssetFileInfo_jclass_meta;

bool    AssetFileInfo_calcInfo(jstring pFileName);
jstring AssetFileInfo_getPackagePath();
long    AssetFileInfo_getStartOffset();
long    AssetFileInfo_getLength();

#endif