#ifndef __HTTP_REACTOR_TIMEOUT_TIMER_H_INCLUDED_
#define __HTTP_REACTOR_TIMEOUT_TIMER_H_INCLUDED_

#include <sys/time.h>

extern struct timeval time_now;

int timer_cmp(struct timeval *, struct timeval *);

int timer_add(const struct timeval *, const struct timeval *, struct timeval *);

int timer_sub(const struct timeval *, const struct timeval *, struct timeval *);

struct timeval* timer_now();

int timer_isset(struct timerval *tv);

void timer_reset(struct timeval *tv);
#endif

