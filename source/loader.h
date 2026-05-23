#ifndef SOLOADER_LOADER_H
#define SOLOADER_LOADER_H

#ifdef __cplusplus
extern "C" {
#endif

int  soloader_init();
int  soloader_run();
void soloader_term();

#ifdef __cplusplus
};
#endif

#endif