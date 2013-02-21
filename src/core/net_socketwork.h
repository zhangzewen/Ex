#ifndef _HTTP_NET_SOCKETWORK_H_INCLUDED
#define _HTTP_NET_SOCKETWORK_H_INCLUDED

#include <sys/socket.h>
#include <sys/types.h>

int tcp_connect(const char *host, const char *serv);
int tcp_listen(const char *host, const char *serv, socklen_t *addrlenp);
int Sock_bind_wild(int sockfd, int family);
int sock_cmp_addr(const struct sockaddr *sa1, const struct sockaddr *sa2,socklen_t salen);
#endif
