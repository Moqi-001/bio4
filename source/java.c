#include "java/lang/System.h"

#include "java/Accelerometer.h"
#include "java/AssetFileInfo.h"
#include "java/AudioPlayer.h"
#include "java/Bio4PreActivity.h"
#include "java/DeviceInfo.h"
#include "java/FileManager.h"
#include "java/StereoViewAdapter.h"
#include "java/TextBitmap.h"
#include "java/VideoAdapter.h"

int soloader_setup_java() {
    setup_java_lang_System();

    setup_Accelerometer();
    setup_AssetFileInfo();
    setup_AudioPlayer();
    setup_Bio4PreActivity();
    setup_DeviceInfo();
    setup_FileManager();
    setup_StereoViewAdapter();
    setup_TextBitmap();
    setup_VideoAdapter();

    return 0;
}
