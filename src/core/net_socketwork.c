#include "net_sockwork.h"
#include <netinet/in.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
int Accept(int fd, struct sockaddr *sa, socklen_t *salenptr)
{
	int		n;

again:
	if ( (n = accept(fd, sa, salenptr)) < 0) {
#ifdef	EPROTO
		if (errno == EPROTO || errno == ECONNABORTED)
#else
		if (errno == ECONNABORTED)
#endif
			goto again;
		else
			{}("accept error");
	}
	return(n);
}

void Bind(int fd, const struct sockaddr *sa, socklen_t salen)
{
	if (bind(fd, sa, salen) < 0)
		{}
}

void Connect(int fd, const struct sockaddr *sa, socklen_t salen)
{
	if (connect(fd, sa, salen) < 0)
		{}
}

void Getpeername(int fd, struct sockaddr *sa, socklen_t *salenptr)
{
	if (getpeername(fd, sa, salenptr) < 0)
		{}
}

void Getsockname(int fd, struct sockaddr *sa, socklen_t *salenptr)
{
	if (getsockname(fd, sa, salenptr) < 0)
		{}
}

void Getsockopt(int fd, int level, int optname, void *optval, socklen_t *optlenptr)
{
	if (getsockopt(fd, level, optname, optval, optlenptr) < 0)
		{}
}

void Listen(int fd, int backlog)
{
	char	*ptr;

	if ( (ptr = getenv("LISTENQ")) != NULL)
		backlog = atoi(ptr);

	if (listen(fd, backlog) < 0)
		{}
}

ssize_t Recv(int fd, void *ptr, size_t nbytes, int flags)
{
	ssize_t		n;

	if ( (n = recv(fd, ptr, nbytes, flags)) < 0)
		{}
	return(n);
}

ssize_t Recvfrom(int fd, void *ptr, size_t nbytes, int flags,
		 struct sockaddr *sa, socklen_t *salenptr)
{
	ssize_t		n;

	if ( (n = recvfrom(fd, ptr, nbytes, flags, sa, salenptr)) < 0)
		{}
	return(n);
}

ssize_t Recvmsg(int fd, struct msghdr *msg, int flags)
{
	ssize_t		n;

	if ( (n = recvmsg(fd, msg, flags)) < 0)
		{}
	return(n);
}


void Send(int fd, const void *ptr, size_t nbytes, int flags)
{
	if (send(fd, ptr, nbytes, flags) != (ssize_t)nbytes)
		{}
}

void Sendto(int fd, const void *ptr, size_t nbytes, int flags,
	   const struct sockaddr *sa, socklen_t salen)
{
	if (sendto(fd, ptr, nbytes, flags, sa, salen) != (ssize_t)nbytes)
		{}
}

void Sendmsg(int fd, const struct msghdr *msg, int flags)
{
	unsigned int	i;
	ssize_t			nbytes;

	nbytes = 0;	/* must first figure out what return value should be */
	for (i = 0; i < msg->msg_iovlen; i++)
		nbytes += msg->msg_iov[i].iov_len;

	if (sendmsg(fd, msg, flags) != nbytes)
		{}
}

void Setsockopt(int fd, int level, int optname, const void *optval, socklen_t optlen)
{
	if (setsockopt(fd, level, optname, optval, optlen) < 0)
		{}
}

void Shutdown(int fd, int how)
{
	if (shutdown(fd, how) < 0)
		{}
}

int Sockatmark(int fd)
{
	int		n;

	if ( (n = sockatmark(fd)) < 0)
		{}
	return(n);
}

int Socket(int family, int type, int protocol)
{
	int		n;

	if ( (n = socket(family, type, protocol)) < 0)
		{}
	return(n);
}

void Socketpair(int family, int type, int protocol, int *fd)
{
	int		n;

	if ( (n = socketpair(family, type, protocol, fd)) < 0)
		{}
}

int tcp_connect(const char *host, const char *serv)
{
	int				sockfd, n;
	struct addrinfo	hints, *res, *ressave;

	bzero(&hints, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ( (n = getaddrinfo(host, serv, &hints, &res)) != 0)
		err_quit("tcp_connect error for %s, %s: %s",
				 host, serv, gai_strerror(n));
	ressave = res;

	do {
		sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		if (sockfd < 0)
			continue;

		if (connect(sockfd, res->ai_addr, res->ai_addrlen) == 0)
			break;

		Close(sockfd);
	} while ( (res = res->ai_next) != NULL);

	if (res == NULL)	
		{}

	freeaddrinfo(ressave);

	return(sockfd);
}

int tcp_listen(const char *host, const char *serv, socklen_t *addrlenp)
{
	int				listenfd, n;
	const int		on = 1;
	struct addrinfo	hints, *res, *ressave;

	bzero(&hints, sizeof(struct addrinfo));
	hints.ai_flags = AI_PASSIVE;
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ( (n = getaddrinfo(host, serv, &hints, &res)) != 0)
		err_quit("tcp_listen error for %s, %s: %s",
				 host, serv, gai_strerror(n));
	ressave = res;

	do {
		listenfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		if (listenfd < 0)
			continue;

		Setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
		if (bind(listenfd, res->ai_addr, res->ai_addrlen) == 0)
			break;	

		Close(listenfd);
	} while ( (res = res->ai_next) != NULL);

	if (res == NULL)
		{}

	Listen(listenfd, LISTENQ);

	if (addrlenp)
		*addrlenp = res->ai_addrlen;	

	freeaddrinfo(ressave);

	return(listenfd);
}

int sock_cmp_addr(const struct sockaddr *sa1, const struct sockaddr *sa2,
			 socklen_t salen)
{
	if (sa1->sa_family != sa2->sa_family)
		return(-1);

	switch (sa1->sa_family) {
	case AF_INET: {
		return(memcmp( &((struct sockaddr_in *) sa1)->sin_addr,
					   &((struct sockaddr_in *) sa2)->sin_addr,
					   sizeof(struct in_addr)));
	}

#ifdef	IPV6
	case AF_INET6: {
		return(memcmp( &((struct sockaddr_in6 *) sa1)->sin6_addr,
					   &((struct sockaddr_in6 *) sa2)->sin6_addr,
					   sizeof(struct in6_addr)));
	}
#endif

#ifdef	AF_UNIX
	case AF_UNIX: {
		return(strcmp( ((struct sockaddr_un *) sa1)->sun_path,
					   ((struct sockaddr_un *) sa2)->sun_path));
	}
#endif

#ifdef	HAVE_SOCKADDR_DL_STRUCT
	case AF_LINK: {
		return(-1);		/* no idea what to compare here ? */
	}
#endif
	}
    return (-1);
}

int sock_bind_wild(int sockfd, int family)
{
	socklen_t	len;

	switch (family) {
	case AF_INET: {
		struct sockaddr_in	sin;

		bzero(&sin, sizeof(sin));
		sin.sin_family = AF_INET;
		sin.sin_addr.s_addr = htonl(INADDR_ANY);
		sin.sin_port = htons(0);	/* bind ephemeral port */

		if (bind(sockfd, (SA *) &sin, sizeof(sin)) < 0)
			return(-1);
		len = sizeof(sin);
		if (getsockname(sockfd, (SA *) &sin, &len) < 0)
			return(-1);
		return(sin.sin_port);
	}

#ifdef	IPV6
	case AF_INET6: {
		struct sockaddr_in6	sin6;

		bzero(&sin6, sizeof(sin6));
		sin6.sin6_family = AF_INET6;
		sin6.sin6_addr = in6addr_any;
		sin6.sin6_port = htons(0);	/* bind ephemeral port */

		if (bind(sockfd, (SA *) &sin6, sizeof(sin6)) < 0)
			return(-1);
		len = sizeof(sin6);
		if (getsockname(sockfd, (SA *) &sin6, &len) < 0)
			return(-1);
		return(sin6.sin6_port);
	}
#endif
	}
	return(-1);
}

int Sock_bind_wild(int sockfd, int family)
{
	int		port;

	if ( (port = sock_bind_wild(sockfd, family)) < 0)
		{}

	return(port);
}
