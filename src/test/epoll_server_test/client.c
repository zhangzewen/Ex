#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

int main(int argc, char *argv[])
{
#if 0
	struct sockaddr_in client_addr;
	struct sockaddr_in server_addr;
	
	if(argc != 3){
		fprintf(stderr, "Usage:%s<server ip>< server port>", argv[0]);
		exit(1);
	}	
	
	listen_fd = socket(AF_INET, SOCK_STREAM, 0);

	if(listen_fd < 0){
		fprintf(stderr, "create socket error!");
		exit(1);
	}
	
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(atoi(argv[2]));
	inet_pton(AF_INET, argv[1], &server_addr.sin_addr);
	
#endif
	return 0;
}

# if 0

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
	
	epfd = event_init();
	
	e = event_set(epfd, listen_fd, EPOLLIN, listen_callback, NULL);	
	
	event_dispatch_loop(epfd);
	
	return 0;
}


#endif
