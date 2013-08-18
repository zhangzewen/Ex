#ifndef __EVENT_BASE_H_INCLUDED__
#define __EVENT_BASE_H_INCLUDED__

#include <sys/time.h>

#include "list.h"

#define EVLIST_TIMOUT 0X01
#define EVLIST_INSERTED 0X02
#define EVLIST_SIGNAL 0X04
#define EVLIST_ACTIVE 0X08
#define EVLIST_INTERNAL 0X10
#define EVLIST_INIT 0X80

#define EVLIST_ALL (0Xf000 | 0x9f)

#define EV_TIMEOUT 0X01
#define EV_READ 0X02
#define EV_WRITE 0X03
#define EV_SIGNAL 0X08
#define EV_PERSIST 0X10

#define EVLOOP_NONBLOCK 0x02  /**< Do not block. */
#include <sys/epoll.h>

struct event_base;

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

struct event{
  struct event_base *ev_base;

  int ev_fd;
  short ev_events;
  short ev_ncalls;
  short *ev_pncalls;

  void (*ev_callback)(int, short, void *arg);
  void *ev_arg;

  int ev_res;
  int ev_flags;
  struct list_head list;
};

struct eventop {
	const char *name;
	struct epoll_loop *(*init)(struct event_base *);
	int (*add)(struct epoll_loop *loop, struct event *);
	int (*del)(struct epoll_loop *loop, struct event *);
	int (*dispatch)(struct event_base *, struct epoll_loop *loop);
	void (*dealloc)(struct event_base *, struct epoll_loop *loop);
};

struct event_base {
	struct eventop evsel;
	struct epoll_loop *evbase;
	int event_count;
	int event_count_active;
	
	int event_gotterm;
	int event_break;

	int nactivequeues;
	
	struct list_head activequeue;
	struct list_head eventqueue;
};


#endif
