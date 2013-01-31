#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <netinet/in.h>
#include <sys/epoll.h>

#include "event.h"

int event_init(void)
{
	return epoll_create(8192);
}

void event_destroy(int epfd, int fd, struct event *e)
{
	close(fd);
	free(e);
}

int event_add(struct event *e)
{
	struct epoll_event ev;

	memset(&ev, '\0', sizeof(struct epoll_event));
	ev.events = e->events;
	ev.data.ptr = e;

	if (epoll_ctl(e->event_epfd, EPOLL_CTL_ADD, e->event_fd, &ev) == -1) {
		perror("epoll_ctl");
		goto out;
	}
	return 0;
out:
	return -1;
}

struct event * event_set(int epfd, int fd, uint32_t events, void (*callback)(int, int, struct event *))
{
	struct event *e;

	if ((e = calloc(1, sizeof(struct event))) == NULL) 
		goto out;

	e->event_epfd 	= epfd;
	e->event_fd	= fd;
	e->events	= events;
	e->callback	= callback;

	return e;
out:
	return NULL;
}

int event_dispatch_loop(int epfd)
{
	struct epoll_event ev[1024];
	int count;
	int i;
	struct event *e;

	memset(&ev, '\0', sizeof(ev));
	count = epoll_wait(epfd, ev, 1024, EPOLL_WAIT_TIMEOUT);
	if (count <= 0)
		goto out;

	for (i = 0; i < count; i++) {
		e = (struct event *)ev[i].data.ptr;
		if (e && e->callback) {
			e->events = ev[i].events;	/* current events */
			e->callback(e->event_epfd, e->event_fd, e);
		}
	}
out:
	return count;
}
