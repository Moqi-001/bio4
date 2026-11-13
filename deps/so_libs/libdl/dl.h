#ifndef SOLOADER_DL_H
#define SOLOADER_DL_H

#ifdef __cplusplus
extern "C" {
#endif

void *dlopen_soloader(const char *filename, int flag);
void *dlsym_soloader(void *handle, const char *symbol);
void  dlclose_soloader(void *handle);
char *dlerror_soloader(void);

#ifdef __cplusplus
}
#endif

#endif