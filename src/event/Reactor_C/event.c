#include <stdio.h>
#include "event.h"


struct event_base *current_base = NULL;

static int use_monotonic;

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

	LIST_HEAD_INIT(&base->eventqueue);

	base->evbase = NULL;

	for (i = 0; eventops[i] && !base->evbase; i++) {
		base->evsel = eventops[i];
		
		base->evbase = base->evsel->init(base);
	}

	if(base->evbase == NULL) {
		fprintf(stderr, "%s: no event mechanism available", __func__);
	}

	event_base_priority_init(base, 1);

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
	
	assert(base);
	
	for (ev = list_first_entry(&base->eventqueue); ev;) {
		struct event *next = TAILQ_NEXT(ev, ev_next);
		if(!(ev->ev_flags & EVLIST_INTERNAL)) {
			event_del(ev);
			++n_deleted;
		}
		ev = next;
	}
	
	while ((ev = min_heap_top(&base->timeheap)) != NULL) {
		event_del(ev);
		++n_deleted;
	}

	for (i = 0; i < base->nactivequeues; ++i) {
		for (ev = list_first_entry(base->activequeues[i]); ev;) {
			if (!(ev->ev_flags & EVLIST_INTERNAL)) {
				event_del(ev);
				++n_deleted;
			}
			ev = next;
		}
	}

	if (n_deleted) {
		fprintf(stderr,"%s : %d events were still set in base", __func__, n_deleted);
	}

	if (base->evsel->dealloc != NULL) {
		base->evsel->dealloc(base, base->evbase);
	}

	for (i = 0; i < base->nactivequeues; i++) {
			assert(LIST_EMPTY(base->activequeues[i]));
	}

	assert(min_heap_empty(&base->timeheap));
	min_heap_dtor(&base->timeheap);

	for(i = 0; i < base->nativequeues; ++i) {
		free(base->activequeues[i]);
	}

	free(base->activequeues);

	assert(LIST_EMPTY(&base->eventqueue));
	
	free(base);
}



static void event_process_active(struct event_base *base)
{
	struct event *ev;
	struct event_list *activeq = NULL;
	int i;
	short ncalls;
	
	for (i = 0; i < base->nactivequeues; ++i) {
		if (list_first_entry(base->activequeues[i]) != NULL) {
			activeq = base->activequeeus[i];
			break;
		}
	}

	assert(activeq != NULL);

	for (ev = list_first_entry(activeq); ev; ev = list_first_entry(activeq)) {
		if (ev->ev_events & EV_PERSIST)
			event_queue_remove(base, ev, EVLIST_ACTIVE);
		else
			event_del(ev);
	}
}
