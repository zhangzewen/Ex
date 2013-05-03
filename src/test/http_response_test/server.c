/*
* server.c
*/
#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <strings.h>
#include <string.h>
#include "http_parse.h"

#define SERVER_PORT 8088 
#define BUFFER_SIZE 1024
#define SERVER_ADDR	"192.168.10.65"


#define ERROR_PAGE  "HTTP/1.1 200 OK\r\nServer: Zhangjie_v1\r\nDate: Fri, 03 May 2013 08:19:45 GMT\r\nContent-Length: 11\r\n\r\nI love You!\0"  
int main(int argc, char * argv[])
{
	http_request request;
	request = (http_request) malloc(sizeof(struct http_request_s));
	struct sockaddr_in server_addr;
	int ret = 0;
	bzero(&server_addr, sizeof(server_addr));	
	
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_PORT);
	ret = inet_pton(AF_INET, SERVER_ADDR, &server_addr.sin_addr);
	socklen_t server_addr_length = sizeof(server_addr);
	if(ret <= 0) {
		printf("transform error!\n");
		exit(1);
	}

	int server_socket = socket(AF_INET, SOCK_STREAM, 0);
	
	if(server_socket < 0){
		printf("Create Socket failed!\n");
		exit(1);
	}
	int opt = 1;

	setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));	
	if(bind(server_socket, (struct sockaddr *)&server_addr, server_addr_length) != 0){
		printf("Server Bind port :%d Failed", SERVER_PORT);
		exit(1);
	}


	if(listen(server_socket, 20)){
		printf("Server Listen Failed!\n");
		exit(1);
	}
	
	while(1){
		struct sockaddr_in client_addr;
		socklen_t length = sizeof(client_addr);
		int new_server_socket = accept(server_socket, (struct sockaddr *)&client_addr, &length);
		if(new_server_socket < 0) {
			printf("Server Accept Failed!\n");
			break;
		}

		char buffer[BUFFER_SIZE] = {0};
		bzero(buffer, BUFFER_SIZE);
		read(new_server_socket, buffer, BUFFER_SIZE);
		puts(buffer);
		parse_http_request(buffer, request);
		write(new_server_socket, ERROR_PAGE, strlen(ERROR_PAGE));
	}

	return 0;	
}
