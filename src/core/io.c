#include "io.h"
#include "http_error.h"

#include <sys/ipc.h>

#include <sys/mman.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>

void set_file_flag(int fd, int flags)
{
	int old_flag;
	int val;
	
	if ((old_flag = fcntl(fd, F_GETFL)) < 0) {
		perror("fcntl GETFL error");
	}
	
	old_flag |= flags;
	
	if((val = fcntl(fd, F_SETFL, old_flag)) < 0) {
		perror("fcntl SETFL error");
	}
}

void clear_file_flag(int fd, int flags)
{
	int old_flag;
	
	int val;

	if ((old_flag = fcntl(fd, F_GETFL)) < 0) {
		perror("fcntl GETFL error");
	}
	
	old_flag &= ~flags;
	
	if((val = fcntl(fd, F_SETFL, old_flag)) < 0) {
		perror("fcntl SETFL error");
	}
}

