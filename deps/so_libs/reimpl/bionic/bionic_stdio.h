#ifndef BIONIC_STDIO_H
#define BIONIC_STDIO_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

extern FILE __sF_fake[3];

FILE  *fopen_soloader(const char *filename, const char *mode);
int    fclose_soloader(FILE *stream);
int    fseek_soloader(FILE *stream, long offset, int whence);
long   ftell_soloader(FILE *stream);
size_t fread_soloader(void *ptr, size_t size, size_t nmemb, FILE *stream);
size_t fwrite_soloader(void *ptr, size_t size, size_t nmemb, FILE *stream);
int    fseeko_soloader(FILE *stream, long offset, int whence);
off_t  ftello_soloader(FILE *stream);

#ifdef __cplusplus
};
#endif

#endif