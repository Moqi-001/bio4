#include <stdio.h>

#include <so_common/so_logger.h>

FILE __sF_fake[3];

FILE *fopen_soloader(const char *filename, const char *mode) {
    FILE *result = fopen(filename, mode);
    so_logd("fopen('%s','%s'): %p", filename, mode, result);
    return result;
}

int fclose_soloader(FILE *stream) {
    int result = fclose(stream);
    so_logd("fclose(%p): %d", stream, result);
    return result;
}

int fseek_soloader(FILE *stream, long offset, int whence) {
    int result = fseek(stream, offset, whence);
    so_logd("fseek(%p,%ld,%d): %d", stream, offset, whence, result);
    return result;
}

long ftell_soloader(FILE *stream) {
    long result = ftell(stream);
    so_logd("ftell(%p): %ld", stream, result);
    return result;
}

size_t fread_soloader(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    size_t result = fread(ptr, size, nmemb, stream);
    so_logd("fread(%p,%zu,%zu,%p): %zu", ptr, size, nmemb, stream, result);
    return result;
}

size_t fwrite_soloader(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    size_t result = fwrite(ptr, size, nmemb, stream);
    so_logd("fwrite(%p,%zu,%zu,%p): %zu", ptr, size, nmemb, stream, result);
    return result;
}

int fseeko_soloader(FILE *stream, long offset, int whence) {
    int result = fseeko(stream, offset, whence);
    so_logd("fseeko(%p,%ld,%d): %d", stream, offset, whence, result);
    return result;
}

off_t ftello_soloader(FILE *stream) {
    long result = ftello(stream);
    so_logd("ftello(%p): %ld", stream, result);
    return result;
}
