#ifndef REACTOR_TIMEOUT_C_EVUTIL_H_INCLUDED__
#define REACTOR_TIMEOUT_C_EVUTIL_H_INCLUDED__
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>

void evutil_timeradd(struct timeval *tvp, struct timeval *uvp, struct timeval *vvp);


void evutil_timersub(struct timeval *tvp, struct timeval *uvp, struct timeval *vvp);

#define evutil_timerclear(tvp) (tvp)->tv_sec = (tvp)->tv_usec = 0

int evutil_timercmp(struct timeval *tvp, struct timeval *uvp);


#define evutil_timerisset(tvp) ((tvp)->tv_sec || (tvp)->tv_usec)



int evutil_gettimeofday(struct timeval *tv, struct timezone *tz);
#endif
