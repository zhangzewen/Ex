#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/socket.h>


void server(int sockfd)
{
	int client_fd;
	int status;
	pid_t pid;
	
	while(1) {
		client_fd = accept(sockfd, NULL, NULL);
		if(client_fd < 0) {
			perror("accept error:");
			exit(1);
		}

		if((pid = fork()) < 0) {
			perror("accept error:");
			exit(1);
		} else if (pid == 0) {
			read();
			write();

			close(client_fd);
		} else {
			close(client_fd);
			wait(NULL);
		}
	}
}


int main(int argc, char *argv[])
{
	
}
