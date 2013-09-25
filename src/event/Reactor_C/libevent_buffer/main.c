#include "http_buffer.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define FILE_PATH "test.txt"
#define FILE_PATH_OUT "out.txt"

int main(int argc, char **argv)
{
	
	int fd;
	int fd_out;
	int n = 0;
	int nwrite;
	int i = 0;
	struct http_buffer *buffer;
	fd = open(FILE_PATH, O_RDONLY);
	if(fd < 0) {
		fprintf(stderr, "can't open file %s!\n", FILE_PATH);
		exit(1);
	}
 	
	fd_out = open(FILE_PATH_OUT, O_CREAT|O_WRONLY|O_APPEND, 00700);
	if(fd_out < 0) {
		fprintf(stderr, "can't open file %s!\n", FILE_PATH_OUT);
		exit(1);
	}

	buffer = buffer_new();
	
	if(NULL == buffer) {
		fprintf(stderr, "can't create evbuffer!\n");
	}

	
	while((n = buffer_read(buffer, fd , 8096))!= 0) {
		printf("\n-------------------------%d------------------------------\n", i);
		printf("buffer->off: %6d", buffer->off);
		printf("read : %5d", n);
		nwrite=buffer_write(buffer, fd_out);
		printf("write: %5d", nwrite);
		i++;
		printf("\n-------------------------------------------------------\n");
	}
#if 0	
	evbuffer_read(buffer, fd, 8096);
	evbuffer_read(buffer_1, fd, 8096);
	evbuffer_add_buffer(buffer, buffer_1);
#endif
	return 0;
}
