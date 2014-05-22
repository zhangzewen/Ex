#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/epoll.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include "event_base.h"
#include "http_epoll.h"
#include "event.h"


struct epoll_loop  *epoll_init(void)
{
	int epfd;
	struct epoll_loop *loop;
	
	if((epfd = epoll_create(32000)) == -1) {
		if (errno != ENFILE) {
			fprintf(stderr, "The system limit on the total number of open files has been reached");
		}
	
		if (errno != ENOMEM) {
			fprintf(stderr, "There was insufficient memory to create the kernel object");	
		}

		return NULL;
	}

	if ((loop = (struct epoll_loop *)malloc(sizeof(struct epoll_loop))) == NULL) {
		return (NULL);
	}
	
	loop->epfd = epfd;

	loop->events = malloc(INITIAL_NEVENTS * sizeof(struct epoll_event));
	if (loop->events == NULL) {
		free(loop);
		return (NULL);
	}

	loop->nevents = INITIAL_NEVENTS;

	loop->fds = calloc(INITIAL_NFILES, sizeof(struct event_epoll));

	if(loop->fds == NULL) {
		free(loop->events);
		free(loop);
		return (NULL);
	}

	loop->nfds = INITIAL_NFILES;
	
	return loop;
}


int epoll_recalc(struct epoll_loop *loop, int max)
{
	
	if(max >= loop->nfds) {
		struct event_epoll *fds;
		int nfds;
		
		nfds = loop->nfds;
		while (nfds <= max) {
			nfds <<= 1;
		}
	
		fds = realloc(loop->fds, nfds * sizeof(struct event_epoll));
		if (fds == NULL) {
			return (-1);
		}
		
		loop->fds = fds;
		memset(fds + loop->nfds, 0, (nfds - loop->nfds) * sizeof(struct event_epoll));
		loop->nfds = nfds;
	}
	
	return (0);
}


int epoll_dispatch(struct epoll_loop *loop, struct timeval *tv)
{
	struct epoll_event *events = loop->events;
	struct event_epoll *event_epoll;
	int i;
	int res;
	int timeout = -1;

	if (tv != NULL) {
		timeout = tv->tv_sec * 1000 + (tv->tv_usec + 999) / 1000;
	}

	if (timeout > MAX_EPOLL_TIMEOUT_MSEC) {
		timeout = MAX_EPOLL_TIMEOUT_MSEC;
	}
	
	
	
	res = epoll_wait(loop->epfd, events, loop->nevents, timeout);
	
	if (res == -1) {
		if (errno != EINTR) {
			return (-1);
		}
	}

	for(i = 0; i < res; i++) {
		int what = events[i].events;
		struct event *event_read = NULL;
		struct event *event_write = NULL;
		int fd = events[i].data.fd;
		
		if (fd < 0 || fd >= loop->nfds) {
			continue;
		}

		event_epoll = &loop->fds[fd];
	
		if (what & (EPOLLHUP|EPOLLERR)) {
			event_read = event_epoll->read;
			event_write =  event_epoll->read;
		} else {
			if (what & EPOLLIN) {
				event_read = event_epoll->read;
			}
			
			if (what & EPOLLOUT) {
				event_write = event_epoll->write;
			}
		}
		
		if (!(event_read || event_write)) {
			continue;
		}
		
		if (event_read != NULL) {
			event_active(event_read, EV_READ, 1);
		}
		
		if (event_write != NULL) {
			event_active(event_write, EV_WRITE, 1);
		}
	}
	
	
	if (res == loop->nevents && loop->nevents < MAX_NEVENTS) {
		int new_nevents = loop->nevents * 2;
		struct epoll_event *new_events;
		
		new_events = realloc(loop->events, new_nevents * sizeof(struct epoll_event));
		if (new_events) {
			loop->events = new_events;
			loop->nevents = new_nevents;
		}
	}

	return (0);
}


int epoll_add(struct epoll_loop *loop, struct event *ev)
{
	struct epoll_event  epoll_event = {0, {0}};
	struct event_epoll *event_epoll;

	int fd;
	int op;
	int events;


	fd = ev->ev_fd;
	
	if (fd >= loop->nfds) {
		if (epoll_recalc(loop, fd) == -1) {
			return (-1);
		}
	}

	event_epoll = &loop->fds[fd];

	op = EPOLL_CTL_ADD;
	
	events = 0;
		
	if (event_epoll->read != NULL) { //是不是已经注册的读事件
		events |= EPOLLIN;
		op = EPOLL_CTL_MOD;
	}

	if (event_epoll->write != NULL) { //是不是已经注册的写事件
		events |= EPOLLOUT;
		op = EPOLL_CTL_MOD;
	}

	if (ev->ev_events & EV_READ) {
		events |= EPOLLIN;
	} 
	
	if (ev->ev_events & EV_WRITE) {
		events |= EPOLLOUT;
	}

	epoll_event.data.fd = fd;
	
	epoll_event.events = events;
	
	if (epoll_ctl(loop->epfd, op, ev->ev_fd, &epoll_event) == -1) {
		return (-1);
	}

	if (ev->ev_events & EV_READ) {
		event_epoll->read = ev;
	}
	
	if (ev->ev_events & EV_WRITE) {
		event_epoll->write = ev;
	}

	return (0);
}


int epoll_del(struct epoll_loop *loop, struct event *ev)
{
	struct epoll_event epoll_event = {0, {0}};
	struct event_epoll *event_epoll;

	int fd;
	int events;
	int op;
	int needwritedelete = 1;
	int needreaddelete = 1;
	
	fd = ev->ev_fd;
	
	if (fd >= loop->nfds) {
		return 0;
	}

	event_epoll = &(loop->fds[fd]);

	op = EPOLL_CTL_DEL;

	events = 0;

	if (ev->ev_events & EV_READ) {
		events |= EPOLLIN;
	}

	if (ev->ev_events & EV_WRITE) {
		events |= EPOLLOUT;
	}

	if ((events & (EPOLLIN|EPOLLOUT)) != (EPOLLIN|EPOLLOUT)) {
		if ((events & EPOLLIN) && (event_epoll->write != NULL)) {
			needwritedelete = 0;
			events = EPOLLOUT;
			op = EPOLL_CTL_MOD;
		} else if ((events & EPOLLOUT) && (event_epoll->read != NULL)) {
			needreaddelete = 0;
			events = EPOLLIN;
			op = EPOLL_CTL_MOD;
		}
	}

	epoll_event.events = events;
	epoll_event.data.fd = fd;
	
	if(needreaddelete) {
		event_epoll->read = NULL;
	}

	if (needwritedelete) {
		event_epoll->write = NULL;
	}

	if (epoll_ctl(loop->epfd, op, fd, &epoll_event) == -1) {
		return (-1);
	}
	
	return (0);
}


void epoll_dealloc(struct epoll_loop *loop)
{

	if (loop->fds) {
		free(loop->fds);
	}

	if (loop->events) {
		free(loop->events);
	}

	if (loop->epfd >= 0) {
		close(loop->epfd);
	}

	memset(loop, 0, sizeof(struct epoll_loop));
	
	free(loop);
}
