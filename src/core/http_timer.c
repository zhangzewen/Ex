#include "http_timer.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "http_timer.h"

/* time_now holds current time */
struct timeval time_now = { tv_sec: 0, tv_usec:0 };

/* set a timer to a specific value */
struct timeval timer_dup(struct timeval b)
{
	struct timeval a;

	TIMER_RESET(a);
	a.tv_sec = b.tv_sec;
	a.tv_usec = b.tv_usec;
	return a;
}

/* timer compare */
int timer_cmp(struct timeval a, struct timeval b)
{
	if (a.tv_sec > b.tv_sec)
		return 1;
	if (a.tv_sec < b.tv_sec)
		return -1;
	if (a.tv_usec > b.tv_usec)
		return 1;
	if (a.tv_usec < b.tv_usec)
		return -1;
	return 0;
}

/* timer sub */
struct timeval timer_sub(struct timeval a, struct timeval b)
{
	struct timeval ret;

	TIMER_RESET(ret);
	ret.tv_usec = a.tv_usec - b.tv_usec;
	ret.tv_sec = a.tv_sec - b.tv_sec;

	if (ret.tv_usec < 0)
	{
		ret.tv_usec += TIMER_HZ;
		ret.tv_sec--;
	}

	return ret;
}

/* timer add */
struct timeval timer_add_long(struct timeval a, long b)
{
	struct timeval ret;

	TIMER_RESET(ret);
	ret.tv_usec = a.tv_usec + b % TIMER_HZ;
	ret.tv_sec = a.tv_sec + b / TIMER_HZ;

	if (ret.tv_usec >= TIMER_HZ)
	{
		ret.tv_sec++;
		ret.tv_usec -= TIMER_HZ;
	}

	return ret;
}

int monotonic_gettimeofday(struct timeval * now)
{
	static struct timeval mono_date;
	static struct timeval drift;	/* warning: signed seconds! */
	struct timeval sys_date, adjusted, deadline;

	if (!now)
	{
		errno = EFAULT;
		return -1;
	}

	gettimeofday(&sys_date, NULL);

	/* on first call, we set mono_date to system date */
	if (mono_date.tv_sec == 0)
	{
		mono_date = sys_date;
		drift.tv_sec = drift.tv_usec = 0;
		*now = mono_date;
		return 0;
	}

	/* compute new adjusted time by adding the drift offset */
	adjusted.tv_sec = sys_date.tv_sec + drift.tv_sec;
	adjusted.tv_usec = sys_date.tv_usec + drift.tv_usec;
	if (adjusted.tv_usec >= TIMER_HZ)
	{
		adjusted.tv_usec -= TIMER_HZ;
		adjusted.tv_sec++;
	}

	/* check for jumps in the past, and bound to last date */
	if (adjusted.tv_sec < mono_date.tv_sec ||
	    (adjusted.tv_sec == mono_date.tv_sec &&
	     adjusted.tv_usec < mono_date.tv_usec))
		goto fixup;

	/* check for jumps too far in the future, and bound them to
	 * TIME_MAX_FORWARD_US microseconds.
	 */
	deadline.tv_sec = mono_date.tv_sec + TIME_MAX_FORWARD_US / TIMER_HZ;
	deadline.tv_usec = mono_date.tv_usec + TIME_MAX_FORWARD_US % TIMER_HZ;
	if (deadline.tv_usec >= TIMER_HZ)
	{
		deadline.tv_usec -= TIMER_HZ;
		deadline.tv_sec++;
	}

	if (adjusted.tv_sec > deadline.tv_sec ||
	    (adjusted.tv_sec == deadline.tv_sec &&
	     adjusted.tv_usec >= deadline.tv_usec))
	{
		mono_date = deadline;
		goto fixup;
	}

	/* adjusted date is correct */
	mono_date = adjusted;
	*now = mono_date;
	return 0;

      fixup:
	drift.tv_sec = mono_date.tv_sec - sys_date.tv_sec;
	drift.tv_usec = mono_date.tv_usec - sys_date.tv_usec;
	if (drift.tv_usec < 0)
	{
		drift.tv_usec += TIMER_HZ;
		drift.tv_sec--;
	}
	*now = mono_date;
	return 0;
}

/* current time */
struct timeval timer_now(void)
{
	struct timeval curr_time;
	int old_errno = errno;

	/* init timer */
	TIMER_RESET(curr_time);
	monotonic_gettimeofday(&curr_time);
	errno = old_errno;

	return curr_time;
}

/* sets and returns current time from system time */
struct timeval set_time_now(void)
{
	int old_errno = errno;

	/* init timer */
	TIMER_RESET(time_now);
	monotonic_gettimeofday(&time_now);
	errno = old_errno;

	return time_now;
}

/* timer sub from current time */
struct timeval timer_sub_now(struct timeval a)
{
	return timer_sub(time_now, a);
}

/* print timer value */
void timer_dump(struct timeval a)
{
	unsigned long timer;
	timer = a.tv_sec * TIMER_HZ + a.tv_usec;
	printf("=> %lu (usecs)\n", timer);
}

unsigned long timer_tol(struct timeval a)
{
	unsigned long timer;
	timer = a.tv_sec * TIMER_HZ + a.tv_usec;
	return timer;
}
