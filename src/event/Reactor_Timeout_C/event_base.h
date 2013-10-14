#ifndef __REACTOR_TIMEOUT_C_EVENT_BASE_H_INCLUDED__
#define __REACTOR_TIMEOUT_C_EVENT_BASE_H_INCLUDED__

#include <sys/time.h>
#include "evbuf.h"

#include "list.h"
#include "evutil.h"
/*以下这几个宏定义是给ev_flags标记的，表明事件当前的状态*/
#define EVLIST_TIMEOUT 0X01 /*event在time堆中*/
#define EVLIST_INSERTED 0X02/*event已经在注册事件链表中*/
#define EVLIST_SIGNAL 0X04/*未见使用*/
#define EVLIST_ACTIVE 0X08/*event在激活链表中*/
#define EVLIST_INTERNAL 0X10/*内部使用标记*/
#define EVLIST_INIT 0X80/*event已经被初始化*/

#define EVLIST_ALL (0Xf000 | 0x9f)

#define EV_TIMEOUT 0X01 //=====1
#define EV_READ 0X02   //======10
#define EV_WRITE 0X04  //=====>100
#define EV_SIGNAL 0X08 //=====>1000
#define EV_PERSIST 0X10 //====>10000

#define EVLOOP_NONBLOCK 0x02  /**< Do not block. */
#include <sys/epoll.h>


#define MAX_EPOLL_TIMEOUT_MSEC (35*60*1000)
#define INITIAL_NFILES 32
#define INITIAL_NEVENTS 32
#define MAX_NEVENTS 4096

struct event_base;

typedef struct min_heap
{
    struct event** p;
    unsigned n, a;
} min_heap_t;

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
	unsigned int min_heap_idx;
	struct timeval ev_timeout;

  int ev_fd;
	struct evbuffer *buffer;	
  short ev_events;
  short ev_ncalls;
  short *ev_pncalls;

  void (*ev_callback)(int, short, void *arg);
  void *ev_arg;

  int ev_res;
  int ev_flags;
  struct list_head event_list;
	struct list_head active_list;
};

struct eventop {
	const char *name;
	struct epoll_loop *(*init)(struct event_base *);
	int (*add)(struct epoll_loop *loop, struct event *);
	int (*del)(struct epoll_loop *loop, struct event *);
	int (*dispatch)(struct event_base *, struct epoll_loop *loop, struct timeval *tv);
	void (*dealloc)(struct event_base *, struct epoll_loop *loop);
};

struct event_base {
	struct eventop *evsel;
	struct epoll_loop *evbase;
	int event_count;
	int event_count_active;
	
	int event_gotterm;
	int event_break;
	
	struct min_heap timeheap;
	struct timeval tv_cache;
	struct timeval event_tv;

	int nactivequeues;
	
	struct list_head activequeue;
	struct list_head eventqueue;
};
#if 0
typedef struct min_heap
{
    struct event** p;
    unsigned n, a;
} min_heap_t;
#endif
static inline void           min_heap_ctor(min_heap_t* s);
static inline void           min_heap_dtor(min_heap_t* s);
static inline void           min_heap_elem_init(struct event* e);
static inline int            min_heap_elem_greater(struct event *a, struct event *b);
static inline int            min_heap_empty(min_heap_t* s);
static inline unsigned       min_heap_size(min_heap_t* s);
static inline struct event*  min_heap_top(min_heap_t* s);
static inline int            min_heap_reserve(min_heap_t* s, unsigned n);
static inline int            min_heap_push(min_heap_t* s, struct event* e);
static inline struct event*  min_heap_pop(min_heap_t* s);
static inline int            min_heap_erase(min_heap_t* s, struct event* e);
static inline void           min_heap_shift_up_(min_heap_t* s, unsigned hole_index, struct event* e);
static inline void           min_heap_shift_down_(min_heap_t* s, unsigned hole_index, struct event* e);

int min_heap_elem_greater(struct event *a, struct event *b)
{
    return evutil_timercmp(&a->ev_timeout, &b->ev_timeout, >);
}

void min_heap_ctor(min_heap_t* s) { s->p = 0; s->n = 0; s->a = 0; }
void min_heap_dtor(min_heap_t* s) { if(s->p) free(s->p); }
void min_heap_elem_init(struct event* e) { e->min_heap_idx = -1; }
int min_heap_empty(min_heap_t* s) { return 0u == s->n; }
unsigned min_heap_size(min_heap_t* s) { return s->n; }
struct event* min_heap_top(min_heap_t* s) { return s->n ? *s->p : 0; }

int min_heap_push(min_heap_t* s, struct event* e)
{
    if(min_heap_reserve(s, s->n + 1))
        return -1;
    min_heap_shift_up_(s, s->n++, e);
    return 0;
}

struct event* min_heap_pop(min_heap_t* s)
{
    if(s->n)
    {
        struct event* e = *s->p;
        min_heap_shift_down_(s, 0u, s->p[--s->n]);
        e->min_heap_idx = -1;
        return e;
    }
    return 0;
}

int min_heap_erase(min_heap_t* s, struct event* e)
{
    if(((unsigned int)-1) != e->min_heap_idx)
    {
        struct event *last = s->p[--s->n];
        unsigned parent = (e->min_heap_idx - 1) / 2;
        if (e->min_heap_idx > 0 && min_heap_elem_greater(s->p[parent], last))
             min_heap_shift_up_(s, e->min_heap_idx, last);
        else
             min_heap_shift_down_(s, e->min_heap_idx, last);
        e->min_heap_idx = -1;
        return 0;
    }
    return -1;
}

int min_heap_reserve(min_heap_t* s, unsigned n)
{
    if(s->a < n)
    {
        struct event** p;
        unsigned a = s->a ? s->a * 2 : 8;
        if(a < n)
            a = n;
        if(!(p = (struct event**)realloc(s->p, a * sizeof *p)))
            return -1;
        s->p = p;
        s->a = a;
    }
    return 0;
}

void min_heap_shift_up_(min_heap_t* s, unsigned hole_index, struct event* e)
{
    unsigned parent = (hole_index - 1) / 2;
    while(hole_index && min_heap_elem_greater(s->p[parent], e))
    {
        (s->p[hole_index] = s->p[parent])->min_heap_idx = hole_index;
        hole_index = parent;
        parent = (hole_index - 1) / 2;
    }
    (s->p[hole_index] = e)->min_heap_idx = hole_index;
}

void min_heap_shift_down_(min_heap_t* s, unsigned hole_index, struct event* e)
{
    unsigned min_child = 2 * (hole_index + 1);
    while(min_child <= s->n)
	{
        min_child -= min_child == s->n || min_heap_elem_greater(s->p[min_child], s->p[min_child - 1]);
        if(!(min_heap_elem_greater(e, s->p[min_child])))
            break;
        (s->p[hole_index] = s->p[min_child])->min_heap_idx = hole_index;
        hole_index = min_child;
        min_child = 2 * (hole_index + 1);
	}
    min_heap_shift_up_(s, hole_index,  e);
}

#endif
