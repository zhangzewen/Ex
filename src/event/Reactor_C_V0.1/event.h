#ifndef __EVENT_H_INCLUDED__
#define __EVENT_H_INCLUDED__

#include <sys/time.h>
#include "event_base.h"

#include "list.h"




struct event_base *event_base_new();

struct event_base *event_init();


int event_dispatch();

int event_base_dispatch(struct event_base *);


void event_base_free(struct event_base *);

int event_base_set(struct event_base *, struct event *);

int event_loop(int);

int event_base_loop(struct event_base *, int);



void event_set(struct event *, int , short , void (*)(int, short, void *), void *, void *);


int event_add(struct event *ev, const struct timeval *timeout);

int event_del(struct event *ev);

void event_active(struct event *, int, short);


#define event_initialized(ev) ((ev)->ev_flags & EVLIST_INIT)

const char *event_get_method(void);

#endif
