#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/dir.h>
#include <sys/dirent.h>

#include <so_common/so_logger.h>
#include <so_jni/so_jni.h>

#include "asset_manager.h"

struct AAssetManager {
    char *asset_root;
};

struct AAssetDir {
    char dirPath[1024];
    DIR *dirp;
};

struct AAsset {
    char   filePath[1024];
    FILE  *fp;
    size_t length;
    void  *buffer;
};

static AAssetManager g_assetManager = {0};

AAssetManager *AAssetManager_create() {
    AAssetManager *mgr = &g_assetManager;

    so_logd("AAssetManager_create(): %p", mgr);
    return mgr;
}

AAssetManager *AAssetManager_fromJava(JNIEnv *env, jobject assetManager) {
    AAssetManager *mgr = &g_assetManager;

    so_logd("AAssetManager_fromJava(%p,%p): %p", env, assetManager, mgr);
    return mgr;
}

AAsset *AAssetManager_open(AAssetManager *mgr, const char *filename, int mode) {
    AAsset *asset = (AAsset *)malloc(sizeof(AAsset));
    if (!asset) {
        so_loge("AAssetManager_open(%p,%s,%d): cannot alloc memory!", mgr, filename, mode);
        return NULL;
    }

    snprintf(asset->filePath, sizeof(asset->filePath), "%sassets/%s", DATA_PATH, filename);
    asset->fp = fopen(asset->filePath, "rb");
    if (!asset->fp) {
        so_loge("AAssetManager_open(%p,%s,%d): cannot open file '%s'!", mgr, filename, mode, asset->filePath);
        free(asset);
        return NULL;
    }

    fseek(asset->fp, 0, SEEK_END);
    asset->length = ftell(asset->fp);
    fseek(asset->fp, 0, SEEK_SET);
    asset->buffer = NULL;

    so_logd("AAssetManager_open(%p,%s,%d): %p", mgr, filename, mode, asset);
    return asset;
}

AAssetDir *AAssetManager_openDir(AAssetManager *mgr, const char *dirName) {
    AAssetDir *assetDir = malloc(sizeof(AAssetDir));
    if (!assetDir) {
        so_loge("AAssetManager_openDir(%p,%s): cannot alloc memory!", mgr, dirName);
        return NULL;
    }

    snprintf(assetDir->dirPath, sizeof(assetDir->dirPath), "%sassets/%s", DATA_PATH, dirName);
    assetDir->dirp = opendir(assetDir->dirPath);
    if (!assetDir->dirp) {
        so_loge("AAssetManager_openDir(%p,%s): cannot open dir '%s'!", mgr, dirName, assetDir->dirPath);
        free(assetDir);
        return NULL;
    }

    so_logd("AAssetManager_openDir(%p,%s): %p", mgr, dirName, assetDir);
    return assetDir;
}

const char *AAssetDir_getNextFileName(AAssetDir *assetDir) {
    if (!assetDir) return NULL;

    struct dirent *entry;
    while ((entry = readdir(assetDir->dirp))) {
        if (SCE_SO_ISREG(entry->d_stat.st_mode)) {
            break;
        }
    }

    if (!entry) {
        so_logd("AAssetDir_getNextFileName(%p): end of directory", assetDir);
        return NULL;
    }

    so_logd("AAssetDir_getNextFileName(%p): '%s'", assetDir, entry->d_name);
    return entry->d_name;
}

void AAssetDir_rewind(AAssetDir *assetDir) {
    if (!assetDir) return;

    so_logd("AAssetDir_rewind(%p)", assetDir);
    rewinddir(assetDir->dirp);
}

void AAssetDir_close(AAssetDir *assetDir) {
    if (!assetDir) return;

    so_logd("AAssetDir_close(%p)", assetDir);
    closedir(assetDir->dirp);
    free(assetDir);
}

int AAsset_read(AAsset *asset, void *buf, size_t count) {
    if (!asset) return -1;

    size_t readlen = fread(buf, 1, count, asset->fp);
    so_logd("AAsset_read(%p,%p,%d): %d", asset, buf, count, readlen);
    return readlen;
}

off_t AAsset_seek(AAsset *asset, off_t offset, int whence) {
    if (!asset) return (off_t)-1;

    if (fseek(asset->fp, offset, whence) != 0) {
        so_loge("AAsset_seek(%p,%ld,%d): error!", asset, offset, whence);
        return (off_t)-1;
    }

    off_t pos = (off_t)ftell(asset->fp);
    so_logd("AAsset_seek(%p,%ld,%d): %ld", asset, offset, whence, pos);
    return pos;
}

off64_t AAsset_seek64(AAsset *asset, off64_t offset, int whence) {
    if (!asset) return (off64_t)-1;

    if (fseek(asset->fp, offset, whence) != 0) {
        so_loge("AAsset_seek(%p,%ld,%d): error!", asset, offset, whence);
        return (off64_t)-1;
    }

    off64_t pos = (off64_t)ftell(asset->fp);
    so_logd("AAsset_seek64(%p,%ld,%d): %ld", asset, offset, whence, pos);
    return pos;
}

void AAsset_close(AAsset *asset) {
    if (!asset) return;

    so_logd("AAsset_close(%p)", asset);
    if (asset->buffer) free(asset->buffer);
    fclose(asset->fp);
    free(asset);
}

const void *AAsset_getBuffer(AAsset *asset) {
    if (!asset) return NULL;

    if (!asset->buffer) {
        asset->buffer = malloc(asset->length);
        if (!asset->buffer) {
            so_loge("AAsset_getBuffer(%p): cannot alloc memory!", asset);
            return NULL;
        }

        long oldOffset = ftell(asset->fp);
        fseek(asset->fp, 0, SEEK_SET);
        size_t readlen = fread(asset->buffer, 1, asset->length, asset->fp);
        fseek(asset->fp, oldOffset, SEEK_SET);
        if (readlen != asset->length) {
            free(asset->buffer);
            asset->buffer = NULL;
            so_loge("AAsset_getBuffer(%p): error!", asset);
            return NULL;
        }
    }

    so_logd("AAsset_getBuffer(%p): %p", asset, asset->buffer);
    return asset->buffer;
}

off_t AAsset_getLength(AAsset *asset) {
    if (!asset) return (off_t)-1;

    off_t length = (off_t)asset->length;
    so_logd("AAsset_getLength(%p): %ld", asset, length);
    return length;
}

off64_t AAsset_getLength64(AAsset *asset) {
    if (!asset) return (off64_t)-1;

    off64_t length = (off64_t)asset->length;
    so_logd("AAsset_getLength64(%p): %ld", asset, length);
    return length;
}

off_t AAsset_getRemainingLength(AAsset *asset) {
    if (!asset) return (off_t)-1;

    off_t remainingLength = (off_t)(asset->length - ftell(asset->fp));
    so_logd("AAsset_getRemainingLength(%p): %ld", asset, remainingLength);
    return remainingLength;
}

off64_t AAsset_getRemainingLength64(AAsset *asset) {
    if (!asset) return (off64_t)-1;

    off64_t remainingLength = (off64_t)(asset->length - ftell(asset->fp));
    so_logd("AAsset_getRemainingLength64(%p): %ld", asset, remainingLength);
    return remainingLength;
}

int AAsset_openFileDescriptor(AAsset *asset, off_t *outStart, off_t *outLength) {
    if (!asset) return -1;

    if (outStart) *outStart = 0;
    if (outLength) *outLength = asset->length;

    int fd = open(asset->filePath, O_RDONLY);
    so_logd("AAsset_openFileDescriptor(%p,%p,%p): %d", asset, outStart, outLength, fd);
    return fd;
}

int AAsset_openFileDescriptor64(AAsset *asset, off64_t *outStart, off64_t *outLength) {
    if (!asset) return -1;

    if (outStart) *outStart = 0;
    if (outLength) *outLength = asset->length;

    int fd = open(asset->filePath, O_RDONLY);
    so_logd("AAsset_openFileDescriptor64(%p,%p,%p): %d", asset, outStart, outLength, fd);
    return fd;
}

int AAsset_isAllocated(AAsset *asset) {
    if (!asset) return 0;
    int result = (asset->buffer != NULL);
    so_logd("AAsset_isAllocated(%p): %d", asset, result);
    return result;
}
