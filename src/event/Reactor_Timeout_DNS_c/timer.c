#include "timer.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/time.h>

struct timeval time_now = { tv_sec: 0, tv_usec: 0};

void timer_reset(struct timeval *tv)
{
	tv->tv_sec = 0;
	tv->tv_usec = 0;
}

struct timeval timer_now()
{
	struct timeval current_time;	
	int old_errno = errno;	
	timer_reset(&current_time);

	gettimeofday(&current_time,NULL);
	errno = old_errno;
	return current_time;
}


int timer_cmp(struct timeval time_a, struct timeval time_b)
{
	if (time_a.tv_sec > time_b.tv_sec)
		return 1;
	if (time_a.tv_sec < time_b.tv_sec)
		return 0;
	if (time_a.tv_usec > time_b.tv_usec)
		return 1;
	if (time_a.tv_usec < time_b.tv_usec)
		return 0;
	return 1;
}


void timer_sub(const struct timeval *timer_a, const struct timeval *timer_b, struct timeval *ret)
{
	timer_reset(ret);
	ret->tv_usec = timer_a->tv_usec - timer_b->tv_usec;
	ret->tv_sec = timer_a->tv_sec - timer_b->tv_sec;

	if (ret->tv_usec < 0) {
		ret->tv_usec += 1000000;
		ret->tv_sec--;
	}	
}

void timer_add(const struct timeval *timer_a, const struct timeval *timer_b, struct timeval *ret)
{
	timer_reset(ret);
	ret->tv_usec = timer_a->tv_usec + timer_b->tv_usec;
	ret->tv_sec = timer_a->tv_sec + timer_b->tv_sec;
	
	
	if (ret->tv_usec >= 1000000) {
		ret->tv_usec -= 1000000;
		ret->tv_sec += 1;
	}

}
int timer_isset(const struct timeval *tv)
{
	return tv->tv_sec !=0 || tv->tv_usec != 0;
}


