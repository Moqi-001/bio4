#include <string.h>

#include <sys/unistd.h>

#include <so_common/so_logger.h>

#define PAGE_SIZE 4096

const unsigned int __page_size = PAGE_SIZE;

int getpagesize_soloader(void) {
    return __page_size;
}

int close_soloader(int fd) {
    int result = close(fd);
    so_logd("close(%d): %d", fd, result);
    return result;
}

int chdir_soloader(const char *dir) {
    char buf[1024];
    int  len = strlen(dir);
    if (dir[len - 1] != '/') {  // psvita的chdir需以/结尾
        if (dir == buf) {
            strcat(buf, "/");
        } else {
            strcpy(buf, dir);
            buf[len++] = '/';
            buf[len]   = '\0';
            dir        = buf;
        }
    }

    int result = chdir(dir);
    so_logd("chdir(%s): %d", dir, result);
    return result;
}

int symlink_soloader(const char *target, const char *linkpath) {
    so_logd("symlink(%s, %s): unsupport!", target, linkpath);
    return 0;
}
