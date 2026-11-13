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

off_t lseek_soloader(int fd, off_t offset, int whence) {
    int result = lseek(fd, offset, whence);
    // so_logd("lseek(%d,%ld,%d): %d", fd, offset, whence, result);
    return result;
}

ssize_t read_soloader(int fd, void *buf, size_t count) {
    ssize_t result = read(fd, buf, count);
    // so_logd("read(%d,%p,%d): %d", fd, buf, count, result);
    return result;
}

ssize_t write_soloader(int fd, const void *buf, size_t count) {
    ssize_t result = write(fd, buf, count);
    // so_logd("write(%d,%p,%d): %d", fd, buf, count, result);
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
    so_loge("symlink(%s, %s): unsupport!", target, linkpath);
    return 0;
}
