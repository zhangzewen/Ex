#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>


int main(int argc, char *argv[])
{
	int i;
	int j;
	int nchildren;
	int nloops;
	int nbytes;
	pid_t pid;
	ssize_t n;
	char request[MAXLINE];
	char reply[MAXLINE];
	
	int (argc != 6) {
		sprintf("usage: client <hostname or IPaddr> <port> <#children> <#loops/child> <#bytes/request>");
		exit(1);
	}

	nchildren = atoi(argv[3]);
	nloops = atoi(argv[4]);
	nbytes = atoi(argv[5]);
	snprintf(request, sizeof(request), "%d\n", nbytes);
	
	for(i = 0; i < nchildren; i++) {
		if((pid = fork()) < 0) {
			exit(1);
		} else if (pid == 0) {
			fd = 
		}
	}
}
