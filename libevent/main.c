#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "../event.h"

static void timeout_cb(int fd, short event, void *arg)
{

	printf("timout now!\n");
}

int main(int argc,  char *argv[])
{
	struct event timeout;
	struct timeval tv;
	
	event_init();

	evtimer_set(&timeout, timeout_cb, &timeout);
	
	evutil_timerclear(&tv);
	tv.tv_sec = 2;
	event_add(&timeout, &tv);
	
	event_dispatch();
	return 0;
}
