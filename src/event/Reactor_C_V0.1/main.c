#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "http_epoll.h"
#include "http_buffer.h"
#include "http_parse.h"
#include "http_request.h"

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
//这里面的逻辑和nginx类似，应为这个水平触发，所以不需要重新添加事件（边缘触发需要添加read事件），如有数据会一直触发
//因该封装一个解析函数，
void ServerRead(int fd, short events, void *arg)
{
	struct event *ev = (struct event *)arg;

	http_connection_t *c;
	http_request_t *r;
	
	c = (http_connection_t *)ev->data;
	r = c->r;
	
	int nread = 0;
	
	//nread = read(fd, buff, sizeof(buff) - 1);
	nread = buffer_read(r->buffer, fd, 2);
	printf("\n----------------------------------------\n");
	printf("ev->buffer->off: %d", r->buffer->off);
	parse_http_request_line(r);
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
	http_connection_t *c;
	socklen_t addrlen = sizeof(addr);
	cli_ev = calloc(1, sizeof(struct event));
	int yes = 1;
	int retval;
	
	c = init_connection();

	cfd = accept(fd ,(struct sockaddr *)&addr, &addrlen);
	
	if(cfd == -1) {
		printf("accept(): can not accept client connection");
		return;
	}

	if(SetNoblock(cfd) == -1) {
		close(cfd);
		return;
	}

	event_set(cli_ev, cfd, EV_READ | EV_PERSIST, ServerRead, (void *)cli_ev, (void *)c);
	c->read = cli_ev;
	c->fd = fd;
	c->buffer = buffer_new();
	c->r = init_request();
	
	c->r->buffer = c->buffer;
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
