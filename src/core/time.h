#ifndef _HTTP_TIME_H_INCLUDED_
#define _HTTP_TIME_H_INCLUDED_
int Gettimeofday(struct timeval *tv, struct timezone *tz);
int Settimeofday(struct timeval *tc, const struct timezone *tz);
#endif
#include <sys/time.h>

typedef struct timeval TIMEVAL;

/* Global vars */
extern TIMEVAL time_now;

/* macro utilities */
#define TIME_MAX_FORWARD_US 2000000
#define TIMER_HZ      1000000
#define TIMER_MAX_SEC 1000
#define TIMER_SEC(T) ((T).tv_sec)
#define TIMER_LONG(T) ((T).tv_sec * TIMER_HZ + (T).tv_usec)
#define TIMER_ISNULL(T) ((T).tv_sec == 0 && (T).tv_usec == 0)
#define TIMER_RESET(T) (memset(&(T), 0, sizeof(struct timeval)))

/* prototypes */
extern TIMEVAL timer_now(void);
extern TIMEVAL set_time_now(void);
extern TIMEVAL timer_dup(TIMEVAL b);
extern int timer_cmp(TIMEVAL a, TIMEVAL b);
extern TIMEVAL timer_sub(TIMEVAL a, TIMEVAL b);
extern TIMEVAL timer_add_long(TIMEVAL a, long b);
extern TIMEVAL timer_sub_now(TIMEVAL a);
extern void timer_dump(TIMEVAL a);
extern unsigned long timer_tol(TIMEVAL a);

#endif
