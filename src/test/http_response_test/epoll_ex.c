#include <stdio.h>
#include <stdlib.h>

#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <strings.h>


#define MAXLINE 5
#define OPEN_MAX 100
#define LISTENQ 20	
#define SERV_PORT 8080
#define INFTIM 1000


void setnonblocking(int sock)
{
	int opts;
	opts = fcntl(sock, F_GETFL);

	if(opts < 0){
		perror("fcntl(sock, GETFL)");
		exit(1);
	}


	opts = opts|O_NONBLOCK;

	if(fcntl(sock, F_SETFL, opts) < 0) {
		perror("fcntl(sock, SETFL, opts)");
		exit(1);
	}
}


int main(int argc, char *argv[])
{
	int i;	
	int maxi;
	int listenfd;
	int connfd;
	int sockfd;
	int epfd;
	int nfds;
	int portnumber;

	ssize_t n;

	char line[MAXLINE];
	
	socklen_t client_len;

	if(2 == argc)
	{
		if((portnumber = atoi(argv[1])) < 0) {
			fprintf(stderr, "Usage:%s portnumber\n", argv[0]);
			return 1;
		}
	} 
	
	
	struct epoll_event ev, events[20];
	
	epfd = epoll_create(256);
	
	struct sockaddr_in client_addr;
	struct sockaddr_in server_addr;



	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	bzero(&server_addr, sizeof(server_addr));

	server_addr.sin_family = AF_INET;

	char *local_addr = "127.0.0.1";

	inet_aton(local_addr, &(server_addr.sin_addr));

	server_addr.sin_port = htons(portnumber);

	bind(listenfd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_in));

	ev.data.fd = listenfd;

	ev.events = EPOLLIN| EPOLLET;

	epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &ev);


	listen(listenfd, LISTENQ);

	maxi = 0;


	while(1) {
	
		nfds = epoll_wait(epfd, events, 20, 500);
		
		for(i = 0; i < nfds; i++) {
			if(events[i].data.fd == listenfd)
			{
				connfd = accept(listenfd, (struct sockaddr *)&client_addr, &client_len);
				
				if(connfd < 0) {
					perror("connfd < 0");
					exit(1);
				}

				char *str = inet_ntoa(client_addr.sin_addr);

				printf("accept a connection from %s", str);
		
				ev.data.fd = connfd;
			
				ev.events = EPOLLIN | EPOLLET;

				epoll_ctl(epfd, EPOLL_CTL_ADD, connfd, &ev);
				break;
			}else if(events[i].events & EPOLLIN) {
				printf("\n=================EPOLLIN=================\n");
				
				if ((sockfd = events[i].data.fd) < 0)
						continue;
				
				if((n = read(sockfd, line, MAXLINE)) < 0) {
					if(errno == ECONNRESET) {
						close(sockfd);
						events[i].data.fd = -1;
					}else{
						perror("readline error");
					}
				} else if(n == 0) {
					printf("read: %s\n", line);

					ev.data.fd = sockfd;
					
					ev.events = EPOLLOUT|EPOLLET;
				}
				
			}else if(events[i].events&EPOLLOUT) {
				sockfd = events[i].data.fd;

				write(sockfd, line, n);

				ev.data.fd = sockfd;
				
				
				ev.events = EPOLLIN|EPOLLET;

				epoll_ctl(epfd, EPOLL_CTL_MOD, sockfd ,&ev);
			}
		}	
	}
	

	return 0;
}
