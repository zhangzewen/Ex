
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "http_epoll.h"
#include "event.h"

struct eventop epollops = {
    .name = "epoll",
    .init = epoll_init,
    .add = epoll_add,
    .del = epoll_del,
    .dispatch = epoll_dispatch,
    .dealloc = epoll_dealloc,
};

struct event_base *current_base = NULL;


static void event_queue_insert(struct event_base *, struct event *, int);
static void event_queue_remove(struct event_base *, struct event *, int);

static int event_haveevents(struct event_base *);

static void event_process_actice(struct event_base *);



struct event_base *event_init(void)
{
	struct event_base *base = event_base_new();
	
	if(base != NULL) {
		current_base = base;
	}

	return (base);
}

struct event_base *event_base_new(void)
{
	int i;
	struct event_base *base;
	
	if ((base = calloc(1, sizeof(struct event_base))) == NULL) {
		fprintf(stderr, "%s: calloc", __func__);
	}

	INIT_LIST_HEAD(&base->eventqueue);
	INIT_LIST_HEAD(&base->activequeue);

	base->evbase = NULL;
	base->evsel = epollops;
	base->evbase = base->evsel.init(base);

	if(base->evbase == NULL) {
		fprintf(stderr, "%s: no event mechanism available", __func__);
	}
	return (base);
}

void event_base_free(struct event_base *base)
{
	int i;
	int n_deleted = 0;
	struct event *ev;

	if (base == NULL && current_base) {
		base =  current_base;
	}
	
	if (base == current_base) {
		current_base = NULL;
	}
	

	list_for_each_entry(ev, &base->eventqueue, list) {
		if (!(ev->ev_flags & EVLIST_INTERNAL)) {
			event_del(ev);
			++n_deleted;
		}
	}


	list_for_each_entry(ev, &base->activequeue, list) {
		if (!(ev->ev_flags & EVLIST_INTERNAL)) {
			event_del(ev);
			++n_deleted;
		}
	}

	if (n_deleted) {
		fprintf(stderr,"%s : %d events were still set in base", __func__, n_deleted);
	}

	if (base->evsel.dealloc != NULL) {
		base->evsel.dealloc(base, base->evbase);
	}



	free(base);
}



static void event_process_active(struct event_base *base)
{
	struct event *ev;
	int i;
	short ncalls;
	

	list_for_each_entry(ev, &base->activequeue, list) {
		if (ev->ev_events & EV_PERSIST) {
			event_queue_remove(base, ev, EVLIST_ACTIVE);
		}else{
			event_del(ev);
		}

		ncalls = ev->ev_ncalls;
		ev->ev_pncalls = &ncalls;
		while(ncalls) {
			ncalls--;
			ev->ev_ncalls = ncalls;
			(*ev->ev_callback)((int)ev->ev_fd, ev->ev_res, ev->ev_arg);

			if (base->event_break) {
				return ;
			}
		}
	}
}


int event_dispatch(void)
{
	return (event_loop(0));
}

int event_base_dispatch(struct event_base *event_base)
{
	return (event_base_loop(event_base, 0));
}

const char *event_base_get_method(struct event_base *base)
{
	return (base->evsel.name);
}



int event_loop(int flags)
{
	return event_base_loop(current_base, flags);
}


int event_base_loop(struct event_base *base, int flags)
{
	struct eventop evsel = base->evsel;
	void *evbase = base->evbase;
	struct timeval tv;
	struct timeval *tv_p;
	int res;
	int done;


	done = 0;
	
	while(!done) {
		if (base->event_gotterm) {
			base->event_gotterm = 0;
			break;
		}
		
		if (base->event_break) {
			base->event_break = 0;
			break;
		}

		
		if (!event_haveevents(base)) {
			fprintf(stderr, "%s: no events registered.", __func__);
			return 1;
		}

		res = evsel.dispatch(base, evbase);

		if (res == -1) {
			return -1;
		}

		if (base->event_count_active) {
			event_process_active(base);
			if (!base->event_count_active && flags) {
				done = 1;
			}else if(flags & EVLOOP_NONBLOCK) {
				done = 1;
			}
		}
	}

	fprintf(stderr, "%s: asked to terminate loop.", __func__);

	return 0;
}


int event_add(struct event *ev, const struct timeval *tv)
{
	struct event_base *base = ev->ev_base;
	struct eventop evsel = base->evsel;
	struct epoll_loop *evbase =  base->evbase;

	int res = 0;
	
	if ((ev->ev_events & (EV_READ | EV_WRITE)) &&
		!(ev->ev_flags & (EVLIST_INSERTED | EVLIST_ACTIVE))) {
		res = evsel.add(evbase, ev);

		if (res != -1) {
			event_queue_insert(base, ev, EVLIST_INSERTED);
		}
	}
	
	return (res);
}


void event_set(struct event *ev, int fd, short events, void (*callback)(int, short, void *), void *arg)
{
	ev->ev_base = current_base;
	ev->ev_callback = callback;
	ev->ev_arg = arg;
	ev->ev_fd = fd;
	ev->ev_events = events;
	ev->ev_res = 0;
	ev->ev_flags = EVLIST_INIT;
	ev->ev_ncalls = 0;
	ev->ev_pncalls = NULL;
}


int event_del(struct event *ev)
{
	struct event_base *base;
	
	struct eventop evsel;
	
	void *evbase;
	
	if (ev->ev_base == NULL) {
		return -1;
	}

	base = ev->ev_base;
	evsel = base->evsel;
	evbase = base->evbase;
	
	if (ev->ev_ncalls && ev->ev_pncalls) {
		*ev->ev_pncalls = 0;
	}


	if (ev->ev_flags & EVLIST_ACTIVE) {
		event_queue_remove(base, ev, EVLIST_ACTIVE);
	}

	if (ev->ev_flags & EVLIST_INSERTED) {
		event_queue_remove(base, ev, EVLIST_INSERTED);
		return (evsel.del(evbase, ev));
	}

	return 0;	
}

__attribute__((unused))
void event_active(struct event *ev, int res, short ncalls)
{
	if (ev->ev_flags & EVLIST_ACTIVE) {
		ev->ev_res |= res;
		return ;
	}

	ev->ev_res = res;
	ev->ev_ncalls = ncalls;
	ev->ev_pncalls = NULL;
	event_queue_insert(ev->ev_base, ev, EVLIST_ACTIVE);
}

void event_queue_insert(struct event_base *base, struct event *ev, int queue)
{
	if (ev->ev_flags & queue) {
		if (queue & EVLIST_ACTIVE) {
			return ;
		}
	}
	

	if (~ev->ev_flags & EVLIST_INTERNAL) {
		base->event_count++;
	}
	

	ev->ev_flags |= queue;

	switch(queue) {
		case EVLIST_INSERTED:
			base->event_count++;
			list_add_tail(&ev->list, &base->eventqueue);
			break;
		case EVLIST_ACTIVE:
			base->event_count_active++;
			list_add_tail(&ev->list, &base->activequeue);
			break;
		default:
			fprintf(stderr, "%s: unknown queue %x", __func__, queue);
	}
}



void event_queue_remove(struct event_base *base, struct event *ev, int queue)
{
	if (!(ev->ev_flags & queue)) {
		fprintf(stderr, "%s: %p(fd %d) not no queue %x", __func__, ev, ev->ev_fd, queue);
	}

	if (~ev->ev_flags & EVLIST_INTERNAL) {
		base->event_count--;
	}

	ev->ev_flags &= ~queue;

	switch (queue) {
		case EVLIST_INSERTED:
			list_del(&ev->list);
			break;
		case EVLIST_ACTIVE:
			base->event_count_active--;
			list_del(&ev->list);
			break;

		default:
			fprintf(stderr, "%s: unknown queue %x", __func__, queue);
	}
}



int event_haveevents(struct event_base *base)
{
	return base->event_count > 0;
}
