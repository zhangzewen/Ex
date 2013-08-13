#include <stdin.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/epoll.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

struct event_epoll{
	struct event *read;
	struct event *write;
};

struct epoll_loop{
	struct event_epoll *fds;
	int nfds;
	struct epoll_event *events;
	int nevents;
	int epfd;
};

static void *epoll_init (struct event_base *);
static int *epoll_add (void *, struct event *);
static int epoll_del (void *, struct event *);
static int epoll_dispatch (struct event_base *, void *, struct timeval *);
static void epoll_deslloc (struct event_base *, void *);



#define INITIAL_NFILES 32
#define INITIAL_NEVENTS 32
#define MAX_NEVENTS 4096


static void * epoll_init(struct event_base *base)
{
	int epfd;
	struct epoll_loop *epoll_loop;
	
	if((epfd = epoll_create(32000)) == -1) {
		if (errno != ENOSYS) {
			
		}
		return (NULL);
	}

	if (!(epoll_loop = calloc(sizeof(struct epoll_loop)))) {
		return (NULL);
	}
	
	epoll_loop->epfd = epfd;

	epoll_loop->events = malloc(INITIAL_NEVENTS * sizeof(struct epoll_event));
	if (epoll_loop->events == NULL) {
		free(epoll_loop);
		return (NULL);
	}

	epoll_loop->nevents = INITIAL_NEVENTS;

	epoll_loop->fds = calloc(INITIAL_NFILES, sizeof(struct event_epoll));

	if(epoll_loop->fds == NULL) {
		free(epoll_loop->events);
		free(epoll_loop);
		return (NULL);
	}

	epoll_loop->nfds = INITIAL_NFILES;
	
	return (epoll_loop);
}


static int epoll_recalc(struct event_base *base, void *arg, int max)
{
	struct epoll_loop *epoll_loop = arg;
	
	if(max >= epoll_loop->nfds) {
		struct event_epoll *fds;
		int nfds;
		
		nfds = epoll_loop->nfds;
		while (nfds <= max) {
			nfds <<= 1;
		}
	
		fds = realloc(epoll_loop->fds, nfds * sizeof(struct event_epoll));
		if (fds == NULL) {
			return (-1);
		}
		
		epoll_loop->fds = fds;
		memset(fds + epoll_loop->nfds, 0, (nfds - epoll_loop->nfds) * sizeof(struct event_epoll));
		epoll_loop->nfds = nfds;
	}
	
	return (0);
}


static int epoll_dispatch(struct event_base *base, void *arg, struct timeval *tv)
{
	struct epoll_loop *epoll_loop = arg;
	struct epoll_event *events = epoll_loop->events;
	struct event_epoll *event_epoll;
	int i;
	int res;
	
	res = epoll_wait(epoll_loop->epfd, events, epoll_loop->nevents, timeout);
	
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
		
		if (fd < 0 || fd >= epoll_loop->nfds) {
			continue;
		}

		event_epoll = &epoll_loop->fds[fd];
	
		if (what & (EPOLLHUP|EPOLLERR)) {
			event_read = event_epoll->event_read;
			event_write =  event_epoll->event_read;
		} else {
			if (what & EPOLLIN) {
				event_read = event_epoll->event_read;
			}
			
			if (what & EPOLLOUT) {
				event_write = event_epoll->event_write;
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
	
	
	if (res == epoll_loop->nevents && epoll_loop->nevents < MAX_NEVENTS) {
		int new_nevents = epoll_loop->nevents * 2;
		struct epoll_event *new_events;
		
		new_events = realloc(epoll_loop->events, new_nevents * sizeof(struct epoll_event));
		if (new_events) {
			epoll_loop->events = new_events;
			epoll_loop->nevents = new_nevents;
		}
	}

	return (0);
}


static int epoll_add(void *arg, struct event *ev)
{
	struct epoll_loop *epoll_loop = arg;
	struct epoll_event  epoll_event = {0, {0}};
	struct event_epoll *event_epoll;

	int fd;
	int op;
	int events;


	fd = ev->ev_fd;
	
	if (fd >= epoll_loop->nfds) {
		if (epoll_recalc(ev->ev_base, epoll_loop, fd) == -1) {
			return (-1);
		}
	}

	event_epoll = &epoll_loop->fds[fd];

	op = EPOLL_CLI_ADD;
	
	events = 0;
		
	if (event_epoll->event_read != NULL) {
		events |= EPOLLIN;
		op = EPOLL_CLI_MOD;
	}

	if (ev->ev_events & EV_READ) {
		events |= EPOLLIN;
	} 
	
	if (ev->ev_events & EV_WRITE) {
		events |= EPOLLOUT;
	}

	epoll_event.data.fd = fd;
	
	epoll_event.events = events;
	
	if (epoll_ctl(epoll_loop->epfd, op, ev->ev_fd, &epoll_event) == -1) {
		return (-1);
	}

	if (ev->ev_events & EV_READ) {
		event_epoll->event_read = ev;
	}
	
	if (ev->ev_events & EV_WRITE) {
		event_epoll->event_write = ev;
	}

	return (0);
}


static int epoll_del(void *arg, struct event *ev)
{
	struct epoll_loop *epoll_loop = arg;
	struct epoll_event epoll_event = {0, {0}};
	struct even_epoll *event_epoll;

	int fd;
	int events;
	int op;
	int needwritedelete = 1;
	int needreaddelete = 1;
	
	fd = ev->ev_fd;
	
	if (fd >= epoll_loop->nfds) {
		return (0);
	}

	event_epoll = &epoll_loop->fds[fd];

	op = EPOLL_CTL_DEL;

	events = 0;

	if (ev->ev_events & EV_READ) {
		events |= EPOLLIN;
	}

	if (ev->ev_events & EV_WRITE) {
		events |= EPOLLOUT;
	}

	if ((events & (EPOLLIN|EPOLLOUT)) != (EPOLLIN|EPOLLOUT)) {
		if ((events & EPOLLIN) && event_epoll->event_write != NULL) {
			needwritedelete = 0;
			events = EPOLLIN;
			op = EPOLL_CTL_MOD;
		} else if ((events & EPOLLOUT) && event_epoll->event_read != NULL) {
			needreaddelete = 0;
			events = EPOLLIN;
			op = EPOLL_CTL_MOD;
		}
	}

	epoll_event.events = events;
	epoll_event.data.fd = fd;
	
	if(needreaddelet) {
		event_epoll->event_read = NULL;
	}

	if (needwritedelete) {
		event_epoll->event_write = NULL;
	}

	if (epoll_ctl(epoll_loop->epfd, op, fd. &epoll_event) == -1) {
		return (-1);
	}
	
	return (0);
}


static void epoll_dealloc(struct event_base *base, void *arg)
{
	struct epoll_loop *epoll_loop = arg;
	if (epoll_loop->fds) {
		free(epoll_loop->fds);
	}

	if (epll_loop->events) {
		free(epoll_loop->events);
	}

	if (epoll_loop->epfd >= 0) {
		close(epoll_loop->epfd);
	}

	memset(epoll_loop, 0, sizeof(struct epoll_loop));

	free(epoll_loop);
}
