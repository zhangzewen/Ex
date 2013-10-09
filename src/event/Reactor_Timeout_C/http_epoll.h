#ifndef __REACTOR_TIMEOUT_C_HTTP_EPOLL_H_INCLUDED_
#define __REACTOR_TIMEOUT_C_HTTP_EPOLL_H_INCLUDED_

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

struct epoll_loop *epoll_init (struct event_base *);
int epoll_add (struct epoll_loop *loop, struct event *);
int epoll_del (struct epoll_loop *loop, struct event *);
int epoll_dispatch (struct event_base *, struct epoll_loop *loop);
void epoll_dealloc (struct event_base *, struct epoll_loop *loop);
#endif
