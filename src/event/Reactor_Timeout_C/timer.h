#ifndef __HTTP_REACTOR_TIMEOUT_TIMER_H_INCLUDED_
#define __HTTP_REACTOR_TIMEOUT_TIMER_H_INCLUDED_

#include <sys/time.h>

extern struct timeval time_now;

int timer_cmp(struct timeval *, struct timeval *);

int timer_add(struct timeval *, struct timeval *, struct timeval *);

int timer_sub(struct timeval *, struct timeval *, struct timeval *);

struct time_now();

struct gettime();

int timer_add();

int timer_del();

void timer_reset(struct timeval *tv);
#endif

