#ifndef BIONIC_NETDB_H
#define BIONIC_NETDB_H

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/socket.h>

int getnameinfo_soloader(const struct sockaddr *sa, socklen_t salen, char *host, size_t hostlen, char *serv, size_t servlen, int flags);

#ifdef __cplusplus
};
#endif

#endif