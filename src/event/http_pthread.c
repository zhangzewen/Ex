#include "http_pthread.h"
#include <errno.h>
#include "http_error.h"
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


void destroy_list(struct list_head &head)
{
	if(NULL == head) {
		// doing nothing
	}
	
	while(!list_empty(head)) {
		list_del(&head->prev);	
		free(/*total struct*/);
	}

	list_del_int(head);

	free(/*total struct */);

	head = NULL;
	
}

pthread_task pthread_task_create(void)
{
	pthread_task task;
	task = (struct pthread_task_t *)malloc(sizeof(struct pthread_task_t));
	if (NULL == task)
		error_quit("can not create task");
	task->pthread_id = NULL;
	task->status = HTTP_PTHREAD_UNKNOWN;
	task->arg = NULL;
	task->task_func = NULL;
	INIT_LIST_HEAD(&task->list);
	return task;
}

pthread_pool pool_create(int num)
{
	pthread_pool new_pool;
	new_pool = (struct pthread_pool_t *)malloc(sizeof(struct pthread_pool_t));
	if (NULL == new_pool)
		error_quit("can not create a pthread pool !\n");
	new_pool->limit_pthread_num = num;
	new_pool->current_pthreads = 0;
	pthread_mutex_init(&new_pool->pool_mutex, NULL);	
	pthread_mutex_init(&new_pool->queue_mutex, NULL);	
	pthread_cond_init(&new_pool->queue_cond_ready, NULL);
	INIT_LIST_HEAD(&new_pool->pthread_head);
	INIT_LIST_HEAD(&new_pool->wait_pthread_head);
	return new_pool;
}

void add_task(pthread_pool queue_pool, void *(*task_func)(void *), void *arg ) //添加任务
{
	pthread_task new;
	new = pthread_task_create();
	
	new->task_func = task_func;
	new->arg = arg;
	new->status = HTTP_PTHREAD_READY;
	pthread_create(new->pthread_id, NULL,start_routine, new->arg);	
	pthread_mutex_lock(&queue_pool->queue_mutex);
	list_add_tail(&new->list, &queue_pool->pthread_head);	
	pthread_mutex_unlock(&queue_pool->queue_mutex);
}

void destroy_pthread_pool(pthread_pool pool)
{
	destroy_list(&pool->pthread_head);		
	destroy_list(&pool->wait_pthread_head);		
	pthread_mutex_destroy(&pool->queue_mutex);
	pthread_mutex_destroy(&pool->pool_mutex);
	pthread_cond_destroy(&pool->queue_cond_ready);
	free(pool);
	pool = NULL;	
}

