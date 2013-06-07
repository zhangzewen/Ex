#ifndef __HTTP_EPOLL_INCLUDED_
#define __HTTP_EPOLL_INCLUDED_

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
