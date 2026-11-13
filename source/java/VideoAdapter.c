#include <stdio.h>
#include <string.h>

#include <so_common/so_logger.h>
#include <so_jni/so_jni.h>

#include "config.h"
#include "utils/utils.h"
#include "video_player.h"

static JClass sClass = NULL;

JBoolean start(JString pFileName, JFloat nVolume) {
    char pFilePath[1024];
    char pCachePath[1024];
    snprintf(pCachePath, sizeof(pCachePath), "%s%s.mp4", SO_CACHE_DIR, pFileName);

    if (!file_exists(pCachePath)) {
        snprintf(pFilePath, sizeof(pFilePath), "%s%s.png", SO_ASSETS_DIR, pFileName);
        // 创建 cache
        if (!file_copy(pFilePath, pCachePath)) {
            so_logd("[VideoAdapter] start('%s',%f): failed to create cache!", pFileName, nVolume);
            return JNI_FALSE;
        }
        // 修改 major_brand: M4VP -> mp42
        FILE *fp = fopen(pCachePath, "r+");
        if (fp) {
            const char mp42[] = {'m', 'p', '4', '2'};
            fseek(fp, 8, SEEK_SET);
            fwrite(mp42, 1, 4, fp);
            fclose(fp);
        }
    }

    if (video_start(pCachePath, nVolume) < 0) {
        so_logd("[VideoAdapter] start('%s',%f): failed to start video!", pFileName, nVolume);
        return JNI_FALSE;
    }

    so_logd("[VideoAdapter] start('%s',%f)", pFileName, nVolume);
    return JNI_TRUE;
}

JBoolean nowPlaying() {
    JBoolean result = video_is_playing();
    so_logd("[VideoAdapter] nowPlaying(): %d", result);
    return result;
}

static JSymbol class_symbols[] = {
    // 静态方法
    JAVA_SYMBOL_STATIC_METHOD("start", "(Ljava/lang/String;F)Z", &start),
    JAVA_SYMBOL_STATIC_METHOD("nowPlaying", "()Z", &nowPlaying),
};

static JClassMeta class_meta = {
    .name          = "com/eruption/util/VideoAdapter",
    .super_name    = NULL,
    .symbols       = class_symbols,
    .num_symbols   = sizeof(class_symbols) / sizeof(class_symbols[0]),
    .instance_size = 0,
};

void setup_VideoAdapter() {
    sClass = JavaCreateClass(&class_meta);
}
