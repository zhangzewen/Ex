#include "io.h"
#include "http_error.h"

#include <sys/ipc.h>

#include <sys/mman.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>

int set_file_flag(int fd, int flags)
{
	int old_flag;
	int val;
	
	if ((old_flag = fcntl(fd, F_GETFL)) < 0) {
		perror("fcntl GETFL error");
		return -1;
	}
	
	old_flag |= flags;
	
	if((val = fcntl(fd, F_SETFL, old_flag)) < 0) {
		perror("fcntl SETFL error");
		return -1;
	}

	return 0;
}

int clear_file_flag(int fd, int flags)
{
	int old_flag;
	int val;

	if ((old_flag = fcntl(fd, F_GETFL)) < 0) {
		perror("fcntl GETFL error");
		return -1;
	}
	
	old_flag &= ~flags;
	
	if((val = fcntl(fd, F_SETFL, old_flag)) < 0) {
		perror("fcntl SETFL error");
		return -1;
	}

	return 0;
}

int set_fd_nonblock(int fd)
{
	if(set_file_flag(fd, O_NONBLOCK) == -1) {
		perror("set fd nonblocking error!\n");
		return -1;
	}

	return 0;

}
int set_fd_block(int fd)
{
	if(clear_file_flag(fd, O_NONBLOCK) == -1) {
		perror("set fd block error!\n");
		return -1;
	}
	return 0;
}
