
/*
*client.c
*/
#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <strings.h>
#include <string.h>

#define SERVER_PORT	8088 
#define BUFFER_SIZE 1024
#define SERVER_ADDR	"192.168.10.65"
int main(int argc, char * argv[])
{

	
	struct sockaddr_in client_addr;
	bzero(&client_addr, sizeof(client_addr));
	client_addr.sin_family = AF_INET;
	client_addr.sin_addr.s_addr = htons(INADDR_ANY);
	client_addr.sin_port = htons(0);


	int client_socket = socket(AF_INET, SOCK_STREAM, 0);
	
	if(client_socket < 0) {
		printf("Create Socket failed!\n");
		exit(1);
	}

	if(bind(client_socket, (struct sockaddr *)&client_addr,sizeof(client_addr))){
		printf("Client Bind port :%d Failed", SERVER_PORT);
		exit(1);
	}
	
	struct sockaddr_in server_addr;
	
	bzero(&server_addr, sizeof(server_addr));

	server_addr.sin_family = AF_INET;

	if(inet_pton(AF_INET, SERVER_ADDR, &server_addr.sin_addr) <= 0) {
		printf("Server ip address error!\n");
		exit(1);
	}

	server_addr.sin_port = htons(SERVER_PORT);
	socklen_t server_addr_length = sizeof(server_addr);
	
	if(connect(client_socket, (struct sockaddr*)&server_addr, server_addr_length) < 0){
			printf("Can not connet to %s!\n",SERVER_ADDR );
			exit(1);
	}
	char buff[1024] = {0};
	write(client_socket, "nihao", 10);	
	read(client_socket, buff, 1024);	
	return 0;	
}
