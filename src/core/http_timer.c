#include "http_timer.h"
#include "http_error.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

struct timeval time_now = { tv_sec: 0, tv_usec: 0};
int Gettimeofday(struct timeval *tv, struct timezone *tz)
{
	if(gettimeofday(tv,tz) == -1)
		error_sys("gettimeofday error");
	return  0;
}

int Settimeofday(const struct timeval *tv, const struct timezone *tz)
{
	if(settimeofday(tv, tz) == -1)	
		error_sys("settimeofday error");
	return 0;
}

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

	Gettimeofday(&current_time,NULL);
	errno = old_errno;
	return current_time;
}

struct timeval set_time_now()
{
	int old_errno = errno;
	
	timer_reset(time_now);
	Gettimeofday(&time_now, NULL);
	errno = old_errno;
	return time_now;
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

struct timeval timer_dup(struct timeval time_b)
{
	struct timeval time_a;
	timer_reset(&time_a);	
	time_a.tv_sec = time_b.tv_sec;
	time_a.tv_usec = time_b.tv_usec;
	return time_a;
}

