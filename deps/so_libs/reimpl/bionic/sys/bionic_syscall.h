#ifndef BIONIC_SYS_SYSCALL_H_
#define BIONIC_SYS_SYSCALL_H_

#ifdef __cplusplus
extern "C" {
#endif

int syscall_soloader(int number, ...);

#ifdef __cplusplus
};
#endif

#endif