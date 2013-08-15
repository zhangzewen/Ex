#ifndef __HTTP_EPOLL_H_INCLUDED_
#define __HTTP_EPOLL_H_INCLUDED_

#include <sys/types.h>
#include <sys/time.h>
#include <sys/epoll.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#define INITIAL_NFILES 32
#define INITIAL_NEVENTS 32
#define MAX_NEVENTS 4096

#include "event_base.h"

struct event_epoll{
	struct event *read;
	struct event *write;
};

struct epoll_loop{
	struct event_epoll *fds;
	int nfds;
	struct epoll_event *events;
	int nevents;
	int epfd;
};

void *epoll_init (struct event_base *);
int epoll_add (void *, struct event *);
int epoll_del (void *, struct event *);
int epoll_dispatch (struct event_base *, void *);
void epoll_dealloc (struct event_base *, void *);
#endif
