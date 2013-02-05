#ifndef _HTTP_TIMER_H_INCLUDED_
#define _HTTP_TIMER_H_INCLUDED_
#include <time.h>
#if 0
int Gettimeofday(struct timeval *tv, struct timezone *tz);
int Settimeofday(struct timeval *tc, const struct timezone *tz);
#endif


extern struct timeval time_now;

/* macro utilities */
#define TIME_MAX_FORWARD_US 2000000
#define TIMER_HZ      1000000
#define TIMER_MAX_SEC 1000
#define TIMER_SEC(T) ((T).tv_sec)
#define TIMER_LONG(T) ((T).tv_sec * TIMER_HZ + (T).tv_usec)
#define TIMER_ISNULL(T) ((T).tv_sec == 0 && (T).tv_usec == 0)
#define TIMER_RESET(T) (memset(&(T), 0, sizeof(struct timeval)))

/* prototypes */
extern struct timeval timer_now(void);
extern struct timeval set_time_now(void);
extern struct timeval timer_dup(struct timeval b);
extern int timer_cmp(struct timeval a, struct timeval b);
extern struct timeval timer_sub(struct timeval a, struct timeval b);
extern struct timeval timer_add_long(struct timeval a, long b);
extern struct timeval timer_sub_now(struct timeval a);
extern void timer_dump(struct timeval a);
extern unsigned long timer_tol(struct timeval a);

#endif
