#ifndef BIONIC_UNISTD_H
#define BIONIC_UNISTD_H

#ifdef __cplusplus
extern "C" {
#endif

extern const unsigned int __page_size;

int getpagesize_soloader(void);

int     close_soloader(int fd);
off_t   lseek_soloader(int fd, off_t offset, int whence);
ssize_t read_soloader(int fd, void *buf, size_t count);
ssize_t write_soloader(int fd, const void *buf, size_t count);

int chdir_soloader(const char *dir);

int symlink_soloader(const char *target, const char *linkpath);

#ifdef __cplusplus
};
#endif

#endif