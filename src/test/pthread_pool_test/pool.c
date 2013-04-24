#include <stdio.h>
#include <stdlib.h>
#include "http_pthread.h"

int main(int argc, char *argv[])
{
	thread_pool pool = NULL;
	task_queue queue = NULL;
	//thread_task task;
	thread_t thread;
#if 0
	int i = 0;
	for(i = 0; i < 10; i++)
	{
	}
#endif
	pool = thread_pool_create();
	queue = task_queue_create();
	thread = thread_create(NULL, start_routine, (void *)queue);
	while(1){
	}
	return 0;
}
