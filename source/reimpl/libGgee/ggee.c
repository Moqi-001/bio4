#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "ggee.h"
#include "utils/logger.h"

int (*ggeeLibExpand)(void *compressed_data, uint32_t compressed_size, void *expanded_data, uint32_t *expanded_size);

int ggeeLibExpandFile(const char *srcpath, const char *dstpath) {
    int      res             = 0;
    FILE    *handle          = NULL;
    void    *compressed_data = NULL;
    uint32_t compressed_size;
    void    *expanded_data = NULL;
    uint32_t expanded_size;

    handle = fopen(srcpath, "rb");
    if (!handle) {
        l_debug("fopen srcpath failed: %s\n", srcpath);
        goto FAILED;
    }

    fseek(handle, 0, SEEK_END);
    compressed_size = ftell(handle);
    fseek(handle, 0, SEEK_SET);

    compressed_data = malloc(compressed_size);
    if (!compressed_data) {
        l_debug("malloc compressed_data failed!\n");
        goto FAILED;
    }

    fread(compressed_data, 1, compressed_size, handle);
    fclose(handle);

    if (!ggeeLibExpand(compressed_data, compressed_size, &expanded_data, &expanded_size)) {
        l_debug("ggeeLibExpand failed!\n");
        goto FAILED;
    }

    handle = fopen(dstpath, "wb");
    if (!handle) {
        l_debug("fopen dstpath failed: %s\n", srcpath);
        goto FAILED;
    }
    fwrite(expanded_data, 1, expanded_size, handle);

END:
    if (handle)
        fclose(handle);
    if (compressed_data)
        free(compressed_data);
    if (expanded_data)
        free(expanded_data);
    return res;

FAILED:
    res = -1;
    goto END;
}
