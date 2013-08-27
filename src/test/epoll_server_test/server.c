#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "http_epoll.h"
#include "file.h"
#include "io.h"

#define RESPONSE_HTTP "HTTP/1.1 OK 200\r\nServer: zhangjie_v0.1\r\n\r\n zhangjie, welcome to UNIX !\n"

void connfd_callback(int epfd, int epoll_fd, struct event *ev)
{
	struct sockaddr_in *addr;
	char buff[10] = {0};
	addr = (struct sockaddr_in *)ev->arg;
	int n = 0;
	char *ip;
	ip = inet_ntoa(addr->sin_addr);
	
	n = read(epoll_fd, buff, 10);
	if(n < 0){
		event_destroy(epfd, epoll_fd, ev);
		return ;
	}
	printf("\nfrom ip:%s =========>%s\r\nwith epoll_fd=0x%x\n", ip, buff, epoll_fd);
	write(epoll_fd, RESPONSE_HTTP, strlen(RESPONSE_HTTP));
	return;
}

void listen_callback(int epfd, int epoll_fd, struct event *ev)
{
	struct event *tmp;
	int conn_fd;
	struct sockaddr_in *client_addr;
	socklen_t len;
	len = sizeof(struct sockaddr);
	
	client_addr = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in));
	if(client_addr == NULL){
		return;
	}
		
	if((conn_fd = accept(epoll_fd, (struct sockaddr *)client_addr, &len)) < 0) {
			fprintf(stderr, "accept error!");
			return ;
	}
	
	set_fd_nonblock(conn_fd);
	
	tmp = event_set(epfd, conn_fd, EPOLLIN, connfd_callback, (void *)&client_addr);
	
	if(NULL == tmp) {
		close(conn_fd);
	}	
	
	if(event_add(tmp) == -1){
		free(tmp);
		close(conn_fd);
	}
		
	return ;
}



int main(int argc, char *argv[])
{
	int listen_fd;
	int epfd;
	
	struct event *e;
	
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
	
	epfd = event_init();
	
	e = event_set(epfd, listen_fd, EPOLLIN, listen_callback, NULL);	
	
	if( NULL == e){
		close(listen_fd);
	}

	if(event_add(e) == -1){
		free(e);
		close(listen_fd);
	}
	
	
	while(1){
		event_dispatch_loop(epfd);
	}
	
	return 0;
}
