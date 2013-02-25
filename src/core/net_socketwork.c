#include "net_socketwork.h"
#include "Define_Macro.h"
#include "io.h"
//#include "addrinfo.h"
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
int tcp_connect(const char *host, const char *serv)
{
	int				sockfd, n;
	struct addrinfo	hints, *res, *ressave;

	bzero(&hints, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ( (n = getaddrinfo(host, serv, &hints, &res)) != 0)
	{}
#if 0
		err_quit("tcp_connect error for %s, %s: %s",
				 host, serv, gai_strerror(n));
#endif
	ressave = res;

	do {
		sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		if (sockfd < 0)
			continue;

		if (connect(sockfd, res->ai_addr, res->ai_addrlen) == 0)
			break;

		close(sockfd);
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
#if 0
		err_quit("tcp_listen error for %s, %s: %s",
				 host, serv, gai_strerror(n));
#endif
	ressave = res;

	do {
		listenfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		if (listenfd < 0)
			continue;

		setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
		if (bind(listenfd, res->ai_addr, res->ai_addrlen) == 0)
			break;	

		close(listenfd);
	} while ( (res = res->ai_next) != NULL);

	if (res == NULL)
		{}

	listen(listenfd, LISTENQ);

	if (addrlenp)
		*addrlenp = res->ai_addrlen;	

	freeaddrinfo(ressave);

	return(listenfd);
}
#if 0
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

#endif
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
