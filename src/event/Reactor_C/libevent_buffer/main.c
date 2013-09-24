#include "evbuf.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define FILE_PATH "test.txt"
#define FILE_PATH_OUT "out"

int main(int argc, char **argv)
{
	
	int fd;
	int fd_out;
	int n = 0;
	struct evbuffer *buffer;
	fd = open(FILE_PATH, O_RDONLY);
	if(fd < 0) {
		fprintf(stderr, "can't open file %s!\n", FILE_PATH);
		exit(1);
	}
 	
	fd_out = open(FILE_PATH, O_WRONLY);
	if(fd_out < 0) {
		fprintf(stderr, "can't open file %s!\n", FILE_PATH_OUT);
		exit(1);
	}

	buffer = evbuffer_new();
	
	if(NULL == buffer) {
		fprintf(stderr, "can't create evbuffer!\n");
	}

	
	while((n = evbuffer_read(buffer, fd , 8096))!= 0) {
		evbuffer_write(buffer, fd_out);
	}
	
	return 0;
}
