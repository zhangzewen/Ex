#include "http_pthread.h"
#include <errno.h>
#include "http_error.h"
#include <stdio.h>
#include <stdlib.h>


pthread_task pthread_task_create(void *arg, void *(*fun)(void *arg))
{
	pthread_task task;
	task = (pthread_task)malloc(sizeof(struct pthread_task_t));
	if(NULL == task) {
		return (pthread_task)-1;
	}
	task->arg = (arg == NULL ? NULL : arg); 
	task->task_func =(fun ==  NULL ? NULL : fun);
	task->status = HTTP_PTHREAD_UNKNOWN;
	INIT_LIST_HEAD(&task->list);
	return task;
}

task_queue *http_task_queue_cretae(void)
{
	task_queue *queue;
	pthread_task *task;
	int i = 0;
	
	queue = (struct task_queue *)malloc(sizeof(struct task_queue));

	if (NULL == queue)
		error_quit("can not create task queue!");
	queue->current_tasks = 0;
	queue->max_tasks = 1024;
	queue->limit_task_num = 10;
	queue->increase_step = 4;
	INIT_LIST_HEAD(&queue->task_queue_head);
	
	for(i = 0; i < queue->limit_task_num; i++) {
		task = pthread_task_create(NULL, NULL);
		list_add_tail(&task->list, &queue->task_queue_head);
	}
	return queue;
}

