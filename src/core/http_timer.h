#ifndef _HTTP_HTTP_TIMER_H_INCLUDED_
#define _HTTP_HTTP_TIMER_H_INCLUDED_

#include <sys/time.h>

int Gettimeofday(struct timeval *tv, struct timezone *tz);
int Settimeofday(const struct timeval *tv, const struct timezone *tz);
struct timeval timer_now();
int timer_cmp(struct timeval time_a, struct timeval time_b);
struct timeval timer_dup(struct timeval time_b);
void timer_reset(struct timeval *tv);
#endif

