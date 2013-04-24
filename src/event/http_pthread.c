#include "http_pthread.h"
#include <errno.h>
#include "http_error.h"
#include <stdio.h>
#include <stdlib.h>


/*****************task************************************/
int get_current_tasks_count(task_queue queue)
{
	pthread_mutex_lock(&queue->task_queue_mutex);
	int current_tasks = 0;
	current_tasks = queue->current_tasks;
	pthread_mutex_unlock(&queue->task_queue_mutex);
	return current_tasks;
}
thread_task thread_task_create(void *arg, void *(*fun)(void *arg))
{
	thread_task task;
	task = (thread_task)malloc(sizeof(struct thread_task_s));
	if(NULL == task) {
		return (thread_task)-1;
	}
	task->arg = (arg == NULL ? NULL : arg); 
	task->task_func =(fun ==  NULL ? NULL : fun);
	task->status = HTTP_PTHREAD_UNKNOWN;
	INIT_LIST_HEAD(&task->list);
	return task;
}

task_queue task_queue_create(void)
{
	task_queue queue;
	
	queue = (task_queue )malloc(sizeof(struct task_queue_s));

	if (NULL == queue)
		error_quit("can not create task queue!");
	pthread_mutex_init(&queue->task_queue_mutex, NULL);
	pthread_cond_init(&queue->task_queue_cond, NULL);
	queue->current_tasks = 0;
	queue->max_tasks = 1024;
	queue->limit_task_num = 10;
	queue->increase_step = 4;
	INIT_LIST_HEAD(&queue->task_queue_head);
	
	return queue;
}


int add_task(task_queue queue, thread_task task)
{
	if (NULL == queue) {
		error_quit("the task_queue is empty!\n");
		return -1;
	}

	if (NULL == queue) {	
		error_quit("the task to be added is empty!\n");
		return -1;
	}
	if (get_current_tasks_count(queue) >= queue->max_tasks) {
		return -1;
	}
	list_add_tail(&task->list, &queue->task_queue_head);
	return 0;
}
int destory_task(thread_task task)
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
/********************thread***************************/
int get_current_threads_count(thread_pool pool)
{
	pthread_mutex_lock(&pool->thread_pool_mutex);
	int current_threads = 0;
	current_threads = pool->current_threads;
	pthread_mutex_unlock(&pool->thread_pool_mutex);
	return current_threads;
	
}
void *start_routine(void *arg)
{
	task_queue queue = (task_queue)arg;
	
	struct list_head *tmp;
	struct list_head *pos;
	pthread_t pid ;
	while(1){
		pthread_mutex_lock(&queue->task_queue_mutex);
		while(get_current_tasks_count(queue) == 0) {
			pthread_cond_wait(&queue->task_queue_cond, &queue->task_queue_mutex);
		}
		thread_task task;
		list_for_each_safe(pos, tmp, &queue->task_queue_head) {
			task = list_entry(pos, struct thread_task_s, list);
			if(task->thread_id == NULL) {
				break;
			}
		}
		pid = pthread_self();
		task->thread_id = &pid;
		task->task_func(task->arg);
		pthread_mutex_unlock(&queue->task_queue_mutex);
	}

}
thread_t thread_create(const pthread_attr_t *attr, void *(*start_routine)(void *arg), void *arg)
{
	thread_t thread;
	int ret = 0;	
	thread = (thread_t)malloc(sizeof(struct thread_s));
	
	if (NULL == thread) {
		error_quit("malloc thread error!\n");
		return (thread_t)-1;
	}

	ret = pthread_create(thread->pthread_id,attr, start_routine, arg );	
	if (ret != 0){
		free(thread->pthread_id);
		thread->pthread_id = NULL;
		error_quit("create pthreat error1\n");
	}	
	INIT_LIST_HEAD(&thread->list);	
	return thread;	
}

thread_pool thread_pool_create(void)
{
	thread_pool pool;
	pool = (thread_pool)malloc(sizeof(struct thread_pool_s));
	
	if (NULL == pool) {
		error_quit("malloc pool error1\n");
		return (thread_pool)-1;
	}
	
	pthread_mutex_init(&pool->thread_pool_mutex, NULL);
	pthread_cond_init(&pool->thread_pool_cond, NULL);
	pool->current_threads = 0;
	pool->max_threads = 1024;
	pool->increase_step = 8;
	pool->limit_theads_num = 10;
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
	if (get_current_threads_count(pool) >= pool->max_threads) {
		return -1;
	}	
	list_add_tail(&thread->list, &pool->threads_head);
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

