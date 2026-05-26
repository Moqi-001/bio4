#include <stddef.h>

#include <so_jni/so_jni.h>

#include "java/lang/System.h"

#include "java/AssetFileInfo.h"
#include "java/AudioPlayer.h"
#include "java/Bio4PreActivity.h"
#include "java/DeviceInfo.h"
#include "java/FileManager.h"
#include "java/StereoViewAdapter.h"
#include "java/TextBitmap.h"

int soloader_java_init() {
    JSystem_class_init();

    AssetFileInfo_class_init();
    Bio4PreActivity_class_init();
    DeviceInfo_class_init();
    FileManager_class_init();
    StereoViewAdapter_class_init();
    TextBitmap_class_init();
    AudioPlayer_class_init();

    return 0;
}
