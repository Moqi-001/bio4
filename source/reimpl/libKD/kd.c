#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <kubridge.h>

#include "kd.h"
#include "reimpl/libLD/ld_mem.h"
#include "utils/logger.h"

char *__kdMapFilename(const char *filename, const char *mode) {
    char *new_path = NULL;

    if (filename) {
        new_path = malloc(1024);
        if (!new_path) {
            l_debug("__kdMapFilename(\"%s\",\"%s\") failed: out of memory!", filename, mode);
            return NULL;
        }

        if (filename[0] != '/') {
            snprintf(new_path, 1024, "%s", DATA_PATH, filename);
        } else {
            if (strncmp(filename, "/native/", 8) == 0) {
                snprintf(new_path, 1024, "%s%s", DATA_PATH, filename + 8);
            } else if (strncmp(filename, "/lib/", 5) == 0) {
                snprintf(new_path, 1024, "%sassets/%s", DATA_PATH, filename + 5);
            } else if (strncmp(filename, "/data/", 6) == 0) {
                snprintf(new_path, 1024, "%sRuntimeApps/1760/data/%s", DATA_PATH, filename + 6);
            } else if (strncmp(filename, "/res/", 5) == 0) {
                snprintf(new_path, 1024, "%sRuntimeApps/1760/res/%s", DATA_PATH, filename + 5);
            } else if (strncmp(filename, "/tmp/", 5) == 0) {
                snprintf(new_path, 1024, "%sRuntimeApps/1760/tmp/%s", DATA_PATH, filename + 5);
            } else {
                strcpy(new_path, filename);
            }
        }
    }

    l_debug("__kdMapFilename(\"%s\",\"%s\"): -> %s", filename, mode, new_path);
    return new_path;
}

int __kdFlushICache(const void *ptr, size_t len) {
    l_debug("__kdFlushICache(%p,%d)", ptr, len);
    LdLoadMemContext *context = ldLoadMemGetContextFromData(ptr);
    ldLoadMemFlushCaches(context);
    // kuKernelFlushCaches(ptr, len);
    return 0;
}

int kdExit(void *context) {
    l_debug("kdExit(%p)", context);
    return 0;
}
