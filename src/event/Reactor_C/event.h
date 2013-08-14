#ifndef __EVENT_H_INCLUDED__
#define __EVENT_H_INCLUDED__

struct event{
	struct list_head ev_next;
	struct ev_active_next;

	
	struct event_base *ev_base;
	
	int ev_fd;
	short ev_events;
	short ev_ncalls;
	short *ev_pncalls;
	
	
	int ev_pri;
	
	void (*ev_callback)(int, short, void *arg);
	void *ev_arg;
	
	int ev_res;
	int ev_flags;
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

typedef void (*event_log_cb)(int severity, const char *msg);

void event_set_log_callback(event_log_cb cb);

int event_base_set(struct event_base *, struct event *）；

#define EVLOOP_ONCE 0X01
#define EVLOOP_NONBLOCK 0X02

int event_loop(int);

int event_base_loop(struct event_base *, int);

int event_loopexit(const struct timaval *);

int event_loopbreak();

int event_base_loopbreak(struct event_base *);


void event_set(struct event *, int , short , void (*)(int, short, void *), void *);

int event_once(int, short, void (*)(int, short, void *), void *, const struct timeval *);

int event_base_once(struct event_base *base, int fd, short events, void (*callback)(int, short, void *), void *arg,
	const struct timeval *timeout);

int event_add(struct event *ev, const struct timeval *timeout);

int event_del(struct event *ev);

void event_active(struct event *, int, short);

int event_pending(struct event *ev, short event, struct timeval *tv);

#define event_initialized(ev) ((ev)->ev_flags & EVLIST_INIT)

const char *event_get_method(void);

int event_priority_init(int);

int event_base_priority_init(struct event_base *, int);

int event_priority_set(struct event *, int);

