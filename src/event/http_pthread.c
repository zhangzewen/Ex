#include "http_pthread.h"
#include <errno.h>
#include "http_error.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


/*****************task************************************/

void *task_fun(void *arg)
{
#if 0
	thread_task_t task;
	task = (thread_task_t)arg;
	//printf("the task_id: %d is exe by thread: %ld\n",task->task_id, *task->thread_id );
#endif
	return NULL;
}
int get_current_tasks_count(task_queue_t queue)
{
	return  queue->current_tasks;
}
thread_task_t thread_task_create(void *(*fun)(void *arg), unsigned int num)
{
	thread_task_t task;
	task = (thread_task_t)malloc(sizeof(struct thread_task_st));
	if(NULL == task) {
		return NULL;
	}
	task->task_id = num;
	task->arg =	task; 
	task->task_func =fun;
	task->status = HTTP_PTHREAD_UNKNOWN;
	INIT_LIST_HEAD(&task->list);
	return task;
}

task_queue_t task_queue_create(void)
{
	task_queue_t queue;
	
	queue = (task_queue_t )malloc(sizeof(struct task_queue_st));

	if (NULL == queue)
		error_quit("can not create task queue!");
	pthread_mutex_init(&queue->task_queue_mutex, NULL);
	pthread_cond_init(&queue->task_queue_ready, NULL);
	queue->current_tasks = 0;
	queue->max_tasks = 1024;
	INIT_LIST_HEAD(&queue->task_queue_head);
	
	return queue;
}


int add_task(task_queue_t queue, thread_task_t task)
{
	if (NULL == queue) {
		error_quit("the task_queue is empty!\n");
		return -1;
	}

	if (NULL == queue) {	
		error_quit("the task to be added is empty!\n");
		return -1;
	}
	pthread_mutex_lock(&queue->task_queue_mutex);
	if (get_current_tasks_count(queue) >= queue->max_tasks) {
		pthread_mutex_unlock(&queue->task_queue_mutex);

		return -1;
	}
	list_add_tail(&task->list, &queue->task_queue_head);
	queue->current_tasks++;
	pthread_mutex_unlock(&queue->task_queue_mutex);
	return 0;
}
int destory_task(thread_task_t task)
{
	if (NULL == task){
		error_quit("the task to be deleted is empty!\n");
		return -1;
	}

	list_del(&task->list);
	free(task);
	task = NULL;

	return 0;
}
int delete_task_from_queue(task_queue_t queue, thread_task_t task)
{
	destory_task(task);
	queue->current_tasks--;
	return 0;
}
/********************thread***************************/
int get_current_threads_count(thread_pool_t pool)
{
	return pool->current_threads;
}
void *start_routine(void *arg)
{
	task_queue_t queue = (task_queue_t)arg;
	
	struct list_head *tmp;
	struct list_head *pos;
	thread_task_t task;
	pthread_t pid ;
	while(1){
		pthread_mutex_lock(&queue->task_queue_mutex);
		while(get_current_tasks_count(queue) == 0 || list_empty(&queue->task_queue_head)) {
			
			pthread_cond_wait(&queue->task_queue_ready, &queue->task_queue_mutex);
		}
		list_for_each_safe(pos, tmp, &queue->task_queue_head) {
			task = list_entry(pos, struct thread_task_st, list);
			if(task->thread_id == NULL) {
				break;
			}
		}
		pid = pthread_self();
		task->thread_id = &pid;
		task->task_func(task->arg);
		delete_task_from_queue(queue, task);
		pthread_mutex_unlock(&queue->task_queue_mutex);
		sleep(1);
	}

}
thread_t thread_create(const pthread_attr_t *attr, void *(*start_routine)(void *arg), task_queue_t queue, int i)
{
	thread_t thread;
	int ret = 0;	
	thread = (thread_t)malloc(sizeof(struct thread_st));
	
	if (NULL == thread) {
		error_quit("malloc thread error!\n");
		return NULL;
	}
	thread->pthread_id = (pthread_t *)malloc(sizeof(pthread_t));
	if (NULL == thread->pthread_id) {
		error_quit("malloc error!\n");
	}
	INIT_LIST_HEAD(&thread->list);	
	thread->num = i;

	ret = pthread_create(thread->pthread_id,attr, start_routine, arg );	
	if (ret != 0){
		free(thread->pthread_id);
		thread->pthread_id = NULL;
		error_quit("create pthreat error1\n");
	}	
	return thread;	
}

thread_pool_t thread_pool_create(void)
{
	thread_pool_t pool;
	pool = (thread_pool_t)malloc(sizeof(struct thread_pool_st));
	
	if (NULL == pool) {
		error_quit("malloc pool error1\n");
		return (thread_pool)-1;
	}
	
	pthread_mutex_init(&pool->thread_pool_mutex, NULL);
	pthread_cond_init(&pool->thread_pool_ready, NULL);
	pool->current_threads = 0;
	pool->max_threads = 1024;
	INIT_LIST_HEAD(&pool->threads_head);	
	
	return pool;
}

int add_thread(thread_pool pool, thread_t thread)
{
	if (NULL == pool) {
		error_quit("the pool is empty!\n");
		return -1;
	}

	if (NULL == thread) {
		error_quit("the thread is illegal!\n");
		return -1;
	}
	pthread_mutex_lock(&pool->thread_pool_mutex);
	if (get_current_threads_count(pool) >= pool->max_threads) {
		pthread_mutex_unlock(&pool->thread_pool_mutex);
		return -1;
	}	
	list_add_tail(&thread->list, &pool->threads_head);
	pthread_mutex_unlock(&pool->thread_pool_mutex);
	return 0;
}

int destory_thead(thread_t thread)
{
	if (NULL == thread) {
		error_quit("the thead to be deleted is empty!\n");
		return -1;
	}
	list_del(&thread->list);
	free(thread);
	thread = NULL;

	return 0;
}

int destory_threads_pool(thread_pool pool)
{
	pthread_mutex_lock(&pool->thread_pool_mutex);
	while(1) {
		pthread_cond_wait(&pool->thread_pool_ready, &pool->thread_pool_mutex);
	}
	pthread_mutex_unlock(&pool->thread_pool_mutex);
}
