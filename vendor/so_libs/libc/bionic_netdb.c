#include <netdb.h>

int getnameinfo_soloader(const struct sockaddr *sa, socklen_t salen, char *host, size_t hostlen, char *serv, size_t servlen, int flags) {
    if (host && hostlen > 0) host[0] = '\0';
    if (serv && servlen > 0) serv[0] = '\0';
    return EAI_SYSTEM;
}