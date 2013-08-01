#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "http_pthread.h"
int main(int argc, char *argv[])
{
	thread_pool_t pool; // thread pool
	task_queue_t queue; // task queue

	thread_t thread; // a thread
	thread_task_t task;	// a task
	
	int i = 0; // for threads
	int j = 0; // for tasks


	pool = thread_pool_create();
	queue = task_queue_create();

	//create tasks first

	for (j = 0; j < 20; j++) {
		task = thread_task_create(task_fun, j);
		add_task(queue, task);	
	}	

	for (i = 0; i < 3; i++) {
		thread = thread_create(NULL, start_routine, queue, i);
		add_thread(pool, thread);
	}

	while(1){
	};
}
