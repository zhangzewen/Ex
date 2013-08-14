#ifndef __EVENT_H_INCLUDED__
#define __EVENT_H_INCLUDED__

struct event{
	struct list_head ev_next;
	struct list_head ev_active_next;

	
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

struct event_base {
	const struct eventop *evsel;
	void *evbase;
	int evetn_count;
	int event_count_active;
	
	int event_gotterm;
	int event_break;

	struct list_head **activequeues;
	int nactivequeues;
	
	struct list_head eventqueue;
};

struct eventop {
	const char *name;
	void *(*init)(struct event_base *);
	int (*add)(void *, struct event *);
	int (*del)(void *, struct event *);
	int (*dispatch)(struct event_base *, void *, struct timeval *);
	void (*dealloc)(struct event_base *, void *)'
	int need_reinit;
};

struct event_base *event_base_new();

struct event_base *event_init();

struct event_reinit(struct event_base *base);

int event_dispatch();

int event_base_dispatch(struct event_base *);


void event_base_free(struct event_base *);


#define __EVENT_LOG_DEBUG 0
#define __EVENT_LOG_MSG		1
#define __EVENT_LOG_WARN	2
#define __EVENT_LOG_ERR		3


int event_base_set(struct event_base *, struct event *）；

int event_loop(int);

int event_base_loop(struct event_base *, int);

int event_loopexit(const struct timaval *);

int event_loopbreak();

int event_base_loopbreak(struct event_base *);


void event_set(struct event *, int , short , void (*)(int, short, void *), void *);


int event_add(struct event *ev, const struct timeval *timeout);

int event_del(struct event *ev);

void event_active(struct event *, int, short);


#define event_initialized(ev) ((ev)->ev_flags & EVLIST_INIT)

const char *event_get_method(void);

