#ifndef SOLOADED_KD_H
#define SOLOADED_KD_H

#include <stddef.h>

char *__kdMapFilename(const char *filename, const char *mode);
int   __kdFlushICache(const void *ptr, size_t len);
int   __kdFlushICache(const void *ptr, size_t len);
int   kdExit(void *context);

#endif