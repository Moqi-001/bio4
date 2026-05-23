#include <fcntl.h>
#include <malloc.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <so_common/so_logger.h>

#include "asset_manager.h"

typedef struct aAssetManager {
    int             dummy;  // TODO: mb we will need to store something here in future
    pthread_mutex_t mLock;
} aAssetManager;

typedef struct aAsset {
    char  *filename;
    FILE  *f;
    size_t bytesRead;
    size_t fileSize;
    bool   opened;
} aAsset;

static AAssetManager *g_AAssetManager = NULL;

AAssetManager *AAssetManager_create() {
    if (g_AAssetManager) return g_AAssetManager;

    aAssetManager *m = (aAssetManager *)calloc(1, sizeof(aAssetManager));
    if (!m) {
        so_loge("AAssetManager_create(): cannot alloc memory!");
        return NULL;
    }
    pthread_mutex_init(&m->mLock, NULL);

    g_AAssetManager = (AAssetManager *)m;

    so_logd("AAssetManager_create(): %p", g_AAssetManager);
    return g_AAssetManager;
}

AAssetDir *AAssetManager_openDir(AAssetManager *mgr, const char *dirName) {
    so_loge("AAssetManager_openDir(%p, %s): UNIMPLEMENTED", mgr, dirName);
    return (AAssetDir *)strdup("dummy");
}

AAsset *AAssetManager_open(AAssetManager *mgr, const char *filename, int mode) {
    size_t reallen  = strlen(DATA_PATH) + strlen("assets/") + strlen(filename) + 1;
    char  *realname = (char *)malloc(reallen);
    if (!realname) {
        so_loge("AAssetManager_open(%p, %s, %d): cannot alloc memory!", mgr, filename, mode);
        return NULL;
    }

    snprintf(realname, reallen, "%sassets/%s", DATA_PATH, filename);

    aAsset *a = (aAsset *)calloc(1, sizeof(aAsset));
    if (!a) {
        so_loge("AAssetManager_open(%p, %s, %d): cannot alloc memory!", mgr, filename, mode);
        return NULL;
    }

    a->filename  = realname;
    a->bytesRead = 0;
    a->f         = fopen((const char *)a->filename, "r");

    if (!a->f) {
        so_loge("AAssetManager_open(%p, %s, %d): cannot open file '%s'!", mgr, filename, mode, a->filename);
        free(a->filename);
        free(a);
        return NULL;
    } else {
        fseek(a->f, 0, SEEK_END);
        a->fileSize = ftell(a->f);
        fseek(a->f, 0, SEEK_SET);
        a->opened = true;
    }

    AAsset *asset = (AAsset *)a;

    so_logd("AAssetManager_open(%p, %s, %d): %p", mgr, filename, mode, asset);
    return asset;
}

const char *AAssetDir_getNextFileName(AAssetDir *assetDir) {
    so_loge("AAssetManager_openDir(%p): UNIMPLEMENTED", assetDir);
    return "";
}

void AAssetDir_rewind(AAssetDir *assetDir) {
    so_loge("AAssetDir_rewind(%p): UNIMPLEMENTED", assetDir);
}

void AAssetDir_close(AAssetDir *assetDir) {
    so_loge("AAssetDir_close(%p): UNIMPLEMENTED", assetDir);
    if (assetDir) {
        free(assetDir);
    }
}

int AAsset_read(AAsset *asset, void *buf, size_t count) {
    if (!asset) {
        so_loge("AAsset_getLength(%p): asset is null", asset);
        return -1;
    }

    aAsset *a = (aAsset *)asset;

    if (!a->opened) {
        return -1;
    }

    size_t readlen = fread(buf, 1, count, a->f);
    int    result;

    if (readlen > 0) {
        a->bytesRead += readlen;
        result = (int)readlen;
    } else {
        if (feof(a->f)) {
            result = 0;
        } else {
            result = -1;
        }
    }

    so_logd("AAsset_read(%p, %p, %d): %d", asset, buf, count, result);
    return result;
}

off_t AAsset_seek(AAsset *asset, off_t offset, int whence) {
    if (!asset) {
        so_loge("AAsset_getLength(%p): asset is null", asset);
        return (off_t)-1;
    }

    aAsset *a = (aAsset *)asset;

    if (!a->opened) {
        return -1;
    }

    off_t result = (off_t)fseek(a->f, offset, whence);

    so_logd("AAsset_seek(%p, %d, %d): %ld", asset, offset, whence, result);
    return result;
}

off64_t AAsset_seek64(AAsset *asset, off64_t offset, int whence) {
    return (off64_t)AAsset_seek(asset, offset, whence);
}

void AAsset_close(AAsset *asset) {
    so_logd("AAsset_close(%p)", asset);

    if (asset) {
        aAsset *a = (aAsset *)asset;
        if (a->filename) {
            free(a->filename);
        }
        if (a->opened) {
            fclose(a->f);
        }
        free(a);
    }
}

const void *AAsset_getBuffer(AAsset *asset) {
    so_loge("AAsset_getBuffer(%p): UNIMPLEMENTED", asset);
    return NULL;
}

off_t AAsset_getLength(AAsset *asset) {
    if (!asset) {
        so_loge("AAsset_getLength(%p): asset is null", asset);
        return (off_t)-1;
    }

    aAsset *a = (aAsset *)asset;

    off_t result = (off_t)a->fileSize;

    so_logd("AAsset_getLength(%p): %ld", asset, result);
    return result;
}

off64_t AAsset_getLength64(AAsset *asset) {
    return (off64_t)AAsset_getLength(asset);
}

off_t AAsset_getRemainingLength(AAsset *asset) {
    if (!asset) {
        so_loge("AAsset_getLength(%p): asset is null", asset);
        return (off_t)-1;
    }

    aAsset *a = (aAsset *)asset;

    if (!a->opened) {
        so_loge("AAsset_getRemainingLength(%p): file not opened!", asset);
        return -1;
    }

    off_t result = (off_t)(a->fileSize - a->bytesRead);

    so_logd("AAsset_getRemainingLength(%p): %ld", asset, result);
    return result;
}

off64_t AAsset_getRemainingLength64(AAsset *asset) {
    return (off64_t)AAsset_getRemainingLength(asset);
}

int AAsset_openFileDescriptor(AAsset *asset, off_t *outStart, off_t *outLength) {
    if (!asset) {
        so_loge("AAsset_openFileDescriptor(%p, %p, %p): asset is null", asset, outStart, outLength);
        return -1;
    }

    aAsset *a = (aAsset *)asset;

    if (outStart) *outStart = 0;
    if (outLength) *outLength = a->fileSize;
    if (a->opened) {
        fclose(a->f);
        a->opened = false;
    }

    int result = open(a->filename, O_RDONLY);
    so_logd("AAsset_openFileDescriptor(%p, %p, %p): %d", asset, outStart, outLength, result);
    return result;
}

int AAsset_openFileDescriptor64(AAsset *asset, off64_t *outStart, off64_t *outLength) {
    return AAsset_openFileDescriptor(asset, outStart, outLength);
}

int AAsset_isAllocated(AAsset *asset) {
    so_loge("AAsset_isAllocated(%p): UNIMPLEMENTED", asset);
    return 0;
}
