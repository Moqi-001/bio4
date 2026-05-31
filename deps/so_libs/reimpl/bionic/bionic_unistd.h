#ifndef BIONIC_UNISTD_H
#define BIONIC_UNISTD_H

#ifdef __cplusplus
extern "C" {
#endif

extern const unsigned int __page_size;

int getpagesize_soloader(void);

int close_soloader(int fd);

int chdir_soloader(const char *dir);

int symlink_soloader(const char *target, const char *linkpath);

#ifdef __cplusplus
};
#endif

#endif