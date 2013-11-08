#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>


#include "evbuf.h"
#include "event.h"
#include "event_base.h"

#if 0
static char return_ok[] = "HTTP/1.1 200 OK\r\nHost: 192.168.10.65\r\nConnection: close\r\n\r\n尼玛，终于让老子给你跑通了啊！混蛋！";
	

int SetNoblock(int fd)
{
	int flags;
	
	if((flags = fcntl(fd, F_GETFL)) == -1) {
		return -1;
	}

	if((fcntl(fd, F_SETFL, flags | O_NONBLOCK)) == -1) {
		return -1;
	}

	return 0;
}

void ServerRead(int fd, short events, void *arg)
{
	struct event *ev = (struct event *)arg;
	
	
	int nread = 0;
	
	//nread = read(fd, buff, sizeof(buff) - 1);
	nread = evbuffer_read(ev->buffer, fd, 16);
	printf("\n----------------------------------------\n");
	printf("ev->buffer->off: %d", ev->buffer->off);
	printf("\n----------------------------------------\n");

	if (nread  == -1) {
		event_del(&ev);
	}
	
	write(fd ,return_ok, sizeof(return_ok));
	//close(fd);

//	event_del(&ev);
}

void ServerAccept(int fd, short events, void *arg)
{
	int cfd;
	struct sockaddr_in addr;
	struct event *cli_ev;
	socklen_t addrlen = sizeof(addr);
	cli_ev = calloc(1, sizeof(struct event));
	int yes = 1;
	int retval;

	cfd = accept(fd ,(struct sockaddr *)&addr, &addrlen);
	
	if(cfd == -1) {
		printf("accept(): can not accept client connection");
		return;
	}

	if(SetNoblock(cfd) == -1) {
		close(cfd);
		return;
	}

	event_set(cli_ev, cfd, EV_READ | EV_PERSIST, ServerRead, (void *)cli_ev);
	event_add(cli_ev, NULL);
}


int main(int argc, char *argv[])
{
	int listen_fd;
	int epfd;
	
	event_init();
	struct event ev;
	
	struct sockaddr_in server_addr;
	socklen_t len;
	if(3 != argc) {
		fprintf(stderr, "Usage: %s <server_ip> <server port>", argv[0]);
		exit(1);
	}

	listen_fd = socket(AF_INET, SOCK_STREAM, 0);	
	if(listen_fd < 0) {
		fprintf(stderr, "create socket error!");
		exit(1);
	}

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(atoi(argv[2]));
	inet_pton(AF_INET, argv[1], &server_addr.sin_addr);
	len = sizeof(server_addr);

	if(bind(listen_fd, (struct sockaddr *)&server_addr, len) < 0) {
		fprintf(stderr, "bind error!");
		exit(1);
	}
	
	

	if(listen(listen_fd, 1024) < 0){
		fprintf(stderr, "listen error!\n");
		exit(1);
	}

	int flags = fcntl(listen_fd, F_GETFL);
	flags |= O_NONBLOCK;
	fcntl(listen_fd, F_SETFL, flags);

	event_set(&ev, listen_fd ,EV_READ | EV_PERSIST, ServerAccept, (void *)&ev);

	event_add(&ev, NULL);

	event_dispatch();
	
	return 0;
	
}


#endif

static void timeout_cb1(int fd, short event, void *arg)
{
	printf("1 timeout now!\n");
}

static void timeout_cb2(int fd, short event, void *arg)
{
	printf("2 timeout now!\n");
}

static void timeout_cb3(int fd, short event, void *arg)
{
	printf("3 timeout now!\n");
}

static void timeout_cb4(int fd, short event, void *arg)
{
	printf("4 timeout now!\n");
} 

static void timeout_cb5(int fd, short event, void *arg)
{
	printf("5 timeout now!\n");
}

static void timeout_cb6(int fd, short event, void *arg)
{
	printf("6 timeout now!\n");
}

static void timeout_cb7(int fd, short event, void *arg)
{
	printf("7 timeout now!\n");
}




int main(int argc, char *argv[])
{

	fprintf(stderr, "=====================begin=================\n");
	struct event timeout1;
	struct event timeout2;
#if 0
	struct event timeout3;
	struct event timeout4;
	struct event timeout5;
	struct event timeout6;
	struct event timeout7;
#endif

	struct timeval tv1;
	struct timeval tv2;
#if 0
	struct timeval tv3;
	struct timeval tv4;
	struct timeval tv5;
	struct timeval tv6;
	struct timeval tv7;
#endif
	event_init();
	
	evtimer_set(&timeout1, timeout_cb1, &timeout1);
	evtimer_set(&timeout2, timeout_cb2, &timeout2);
#if 0
	evtimer_set(&timeout3, timeout_cb3, &timeout3);
	evtimer_set(&timeout4, timeout_cb4, &timeout4);
	evtimer_set(&timeout5, timeout_cb5, &timeout5);
	evtimer_set(&timeout6, timeout_cb6, &timeout6);
	evtimer_set(&timeout7, timeout_cb7, &timeout7);
#endif

	tv1.tv_sec = 2;
	tv1.tv_usec = 0;

	tv2.tv_sec = 4;
	tv2.tv_usec = 0;

#if 0
	tv3.tv_sec = 6;
	tv3.tv_usec = 0;

	tv4.tv_sec = 8;
	tv4.tv_usec = 0;

	tv5.tv_sec = 10;
	tv5.tv_usec = 0;

	tv6.tv_sec = 12;
	tv6.tv_usec = 0;

	tv7.tv_sec = 14;
	tv7.tv_usec = 0;
#endif

	event_add(&timeout1, &tv1);
	event_add(&timeout2, &tv2);
#if 0
	event_add(&timeout3, &tv3);
	event_add(&timeout4, &tv4);
	event_add(&timeout5, &tv5);
	event_add(&timeout6, &tv6);
	event_add(&timeout7, &tv7);
#endif
	event_dispatch();

	return 0;
}
