
#ifndef _HTTP_EVENT_H_INCLUDED_
#define _HTTP_EVENT_H_INCLUDED_


#include <stdint.h>

/* 1000 milliseconds */
#define EPOLL_WAIT_TIMEOUT 1000


struct event {
	int event_epfd;		/* epoll fd */
	int event_fd;		/* unix domain socket for script4 communication */
	unsigned int events;	/* current occurs events */
	int closed;		/* indicate already closed */
	void (*callback)(int epfd, int fd, struct event *e);
};


extern int event_init(void);
extern struct event * event_set(int epfd, int fd, uint32_t events, 
				void (*callback)(int, int, struct event *));
extern int event_add(struct event *e);
extern int event_dispatch_loop(int epfd);
extern void event_destroy(int epfd, int fd, struct event *e);

#endif
