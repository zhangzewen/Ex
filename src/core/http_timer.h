#ifndef _HTTP_HTTP_TIMER_H_INCLUDED_
#define _HTTP_HTTP_TIMER_H_INCLUDED_

struct timeval timer_now();
//struct timeval set_time_now();
int timer_cmp(struct timeval time_a, struct timeval time_b);
struct timer_dup(struct timeval time_b);
void timer_reset(struct timeval *time);
int Gettimeofday(strcut timeval *tv, struct timezone *tz);
int Settimeofday(const struct timeval *tv, const struct timezone *tz);
#endif

