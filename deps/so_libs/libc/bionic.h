#ifndef SOLOADER_BIONIC_H
#define SOLOADER_BIONIC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "sys/bionic_atomics.h"
#include "sys/bionic_epoll.h"
#include "sys/bionic_ioctl.h"
#include "sys/bionic_select.h"
#include "sys/bionic_stat.h"
#include "sys/bionic_syscall.h"
#include "sys/bionic_system_properties.h"
#include "sys/bionic_vfs.h"

#include "bionic_ctype.h"
#include "bionic_dirent.h"
#include "bionic_fcntl.h"
#include "bionic_malloc.h"
#include "bionic_md5.h"
#include "bionic_mman.h"
#include "bionic_netdb.h"
#include "bionic_pthread.h"
#include "bionic_semaphore.h"
#include "bionic_stdio.h"
#include "bionic_time.h"
#include "bionic_unistd.h"

#ifdef __cplusplus
};
#endif

#endif