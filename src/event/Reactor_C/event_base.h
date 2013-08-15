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


struct event_base;

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
	void *(*init)(struct event_base *);
	int (*add)(void *, struct event *);
	int (*del)(void *, struct event *);
	int (*dispatch)(struct event_base *, void *, struct timeval *);
	void (*dealloc)(struct event_base *, void *);
};

struct event_base {
	struct eventop evsel;
	void *evbase;
	int event_count;
	int event_count_active;
	
	int event_gotterm;
	int event_break;

	int nactivequeues;
	
	struct list_head activequeue;
	struct list_head eventqueue;
};


#endif
