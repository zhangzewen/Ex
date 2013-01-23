#ifndef _HTTP_TIME_H_INCLUDED_
#define _HTTP_TIME_H_INCLUDED_
int Gettimeofday(struct timeval *tv, struct timezone *tz);
int Settimeofday(struct timeval *tc, const struct timezone *tz);
#endif
