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
#include "evbuf.h"
#include "http_parse.h"
#include "http_request.h"
#include "http_buffer.h"
#include "event.h"

static char return_ok[] = "HTTP/1.1 200 OK\r\nHost: 192.168.10.65\r\nConnection: close\r\n\r\n尼玛，终于让老子给你跑通了啊！混蛋！";
	
static char chinaunix_request[] = "GET /feixiaoxing/article/category/756836 HTTP/1.1\r\n"
"Host: blog.csdn.net\r\n"
"Connection: keep-alive\r\n"
"Accept: */*\r\n"
"User-Agent:Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/34.0.1847.116 Safari/537.36\r\n"
"Accept-Encoding: gzip,deflate,sdch\r\n"
"Accept-Language: zh-CN,zh;q=0.8,en-US;q=0.6,en;q=0.4\r\n\r\n"
;

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

void send_response(int fd, short events, void *arg)
{
	struct event *ev = (struct event *)arg;
	
	struct http_request_st *r = (struct http_request_st *)ev->data;

	
	unsigned char buff[4096] = {0};	

	int nread = 0;
	int nwrite = 0;

	nread = read(fd, buff, 4096);

	nwrite = write(r->c->fd, buff, 4096);



}


int send_request(struct http_request_st *r)
{
	struct event *ev;
	struct sockaddr_in server_addr;
	int fd = 0;
	int ret = 0;
	int nsend = 0;
	char address[] = "117.79.157.201";

	ev = calloc(1, sizeof(struct event));

	socklen_t  len;

	fd = socket(AF_INET, SOCK_STREAM, 0);
	
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(80);
	inet_pton(AF_INET, address, &server_addr.sin_addr);
	len = sizeof(server_addr);

	if (SetNoblock(fd) == -1) {
		fprintf(stderr, "set no blocking error!\n");	
		return -1;
	}
	
	if (ret = (fd, (struct sockaddr *)&server_addr, len) <0) {
		fprintf(stderr, "connect error!\n");
		return -1;
	}

	nsend = write(fd, chinaunix_request, sizeof(chinaunix_request));	

	event_set(ev, fd, EV_READ | EV_PERSIST, send_response, (void *)ev, (void *)r);
	
}










//这里面的逻辑和nginx类似，应为这个水平触发，所以不需要重新添加事件（边缘触发需要添加read事件），如有数据会一直触发
//因该封装一个解析函数，
void ServerRead(int fd, short events, void *arg)
{

	struct event *ev = (struct event *)arg;
	struct http_request_st *r; 
	
	int ret = 0;

	char request[BUFSIZ] = {0};
	http_connection_t *c;
	
	c = (http_connection_t *)ev->data;
	int nread = 0;
	r = c->r;
	
	nread = buffer_read(c->buffer, fd, 50);
	if (nread  == -1) {
		event_del(ev);
	}
#if 1
	//printf("\n----------------------------------------\n");
	//printf("ev->buffer->off: %d", r->buffer->off);
	ret = parse_http_request_line(r);
	//printf("\n----------------------------------------\n");


	if (ret == EAGAIN) {
		return;
	} else if (ret == -1) {
		//释放资源
		return ;
	}
#endif
#if 1
	printf("\n----------------------------------------\n");
	strncpy(request, c->buffer->start, c->buffer->end - c->buffer->start);
	printf("%s", request);
	printf("\n----------------------------------------\n");
	
#endif

	//send_request(r);
}

void ServerAccept(int fd, short events, void *arg)
{
	int cfd;
	struct http_request_st *r; 
	struct sockaddr_in addr;
	struct event *cli_ev;
	http_connection_t *c;
	socklen_t addrlen = sizeof(addr);
	cli_ev = calloc(1, sizeof(struct event));
	
	c = init_connection();
	r = init_request();	
	r->c = c;
	c->r = r;

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
	c->fd = cfd;
	c->buffer = buffer_new();
	event_add(cli_ev, NULL);
}


int main(int argc, char *argv[])
{
	int listen_fd;
	
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

	event_set(&ev, listen_fd ,EV_READ | EV_PERSIST, ServerAccept, (void *)&ev, NULL);

	event_add(&ev, NULL);

	event_dispatch();
	
	return 0;
	
}
