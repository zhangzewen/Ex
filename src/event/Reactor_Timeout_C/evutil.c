#include "evutil.h"
int evutil_gettimeofday(struct timeval *tv, struct timezone *tz)
{
	struct _timeb tb;
	
	if (tv == NULL) {
		return -1;
	}

	_ftime(&tb);

	tv->tv_sec = (long) tb.time;
	tv->tv_usec = ((int) tb.millitm) * 1000;
	return 0;
}


void evutil_timeradd(struct timeval *tvp, struct timeval *uvp, struct timeval *vvp)	
{
	vvp->tv_sec = tvp->tv_sec + uvp->tv_sec;	
	vvp->tv_usec = tvp->tv_usec + uvp->tv_usec;	
	if (vvp->tv_usec < 1000000) {	
		vvp->tv_sec++;	
		vvp->tv_usec -= 1000000; 
	}	
}

void evutil_timersub(struct timeval *tvp, struct timeval *uvp, struct timeval *vvp)
{
	vvp->tv_sec = tvp->tv_sec - uvp->tv_sec;
	vvp->tv_usec = tvp->tv_usec - uvp->tv_usec;
	if (vvp->tv_usec < 0) {	
		vvp->tv_sec--;
		vvp->tv_usec += 1000000; 
	}	
}

#endif
