#include "http_myepoll.h"
#include <sys/epoll.h>


static int ep;
static int connections [4096] = {-1};


struct epoll_event ev;
struct epoll_event events[4096];

int event_init()
{
	ep = epoll_create(4096);
	
	if(ep == -1) {
		if(errno == EINVAL) {
			fprintf(stderr, "argc is not positive");
			return -1;
		}else if(errno == ENFILE) {
			fprintf(stderr, "The fds is out of The system limit!");
			return -1;
		}else if(errno == ENOMEM) {
			fprintf(stderr, "There was insufficient memory to create the kernel object");
			return -1;
		}
	}
	
	return 0;
}



int event_add(int fd , int events)
{
	int ret = 0;
	ev.events = events;
	ev.data.fd = fd;
	
	if(epoll_ctl(ep, EPOLL_CTL_ADD, fd, &ev) < 0){
		fprintf(stderr, "epoll set insertion error:fd = %d\n", fd);
		return -1;
	}
	return 0;
}



int event_del(int epf, int fd , int events)
{
	int ret = 0;
}


void event_process()
{
	while(1){
		nready = epoll_wait(ep, events, 1, -1);
		
		if(nready == -1){
			perror("epoll_wait");
			continue;
		}
		
		for(n = 0; n < nready; n++){
			if(events[n].data.fd == 
		}

	
	}
}
