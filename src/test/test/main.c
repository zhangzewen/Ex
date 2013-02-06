#include <stdio.h>
#include <stdlib.h>
#include "scheduler.h"
#include <unistd.h>
#include <sys/types.h>
int start_child(thread_t * thread)
{
	for(;;)
	{
		sleep(5);
	}
}
int main(int argc, char *argv[])
{
	master = thread_make_master();
	pid_t pid;
	pid = fork();
	if (pid < 0) {
		perror("pid error");
	} else if (pid ) {  // father process
		printf("the father process is pid = %lld\n", (unsigned long long)getpid());	
		wait(NULL);
		return 0;
	}
	thread_add_child(master, start_child, NULL, getpid(), 0);
	return 0;	
}



