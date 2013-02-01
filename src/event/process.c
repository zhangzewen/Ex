#include "process.h"
#include <errno.h>
#include <stdio.h>
pid_t Fork(void)
{
	pid_t pid;
	if((pid = fork()) < 0) {
		perror("fork error");
		return -1;
	}
	return pid;
}
