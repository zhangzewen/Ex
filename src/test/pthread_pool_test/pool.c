#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include "list.h"
typedef struct thread_s{
	struct list_head list;
	pthread_t *pthread_id;
} *thread_t;

typedef struct thread_pool_s{
	pthread_mutex_t thread_pool_mutex;
	pthread_cond_t thread_pool_cond;
	unsigned int current_threads;
	unsigned int max_threads;
	unsigned int increase_step;
	unsigned int limit_theads_num;
	struct list_head threads_head;	
}*thread_pool;


int get_current_threads_count(thread_pool pool)
{
	pthread_mutex_lock(&pool->thread_pool_mutex);
	int current_threads = 0;
	current_threads = pool->current_threads;
	pthread_mutex_unlock(&pool->thread_pool_mutex);
	return current_threads;
	
}
#if 0
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
#endif
thread_t thread_create(void *(*start_routine)(void *arg), void *arg)
{
	thread_t thread;
	int ret = 0;	
	thread = (thread_t)malloc(sizeof(struct thread_s));
	
	if (NULL == thread) {
		perror("malloc thread error!\n");
		return (thread_t)-1;
	}
	INIT_LIST_HEAD(&thread->list);	

	ret = pthread_create(thread->pthread_id, NULL, start_routine, arg );	
	if (ret != 0){
		free(thread->pthread_id);
		thread->pthread_id = NULL;
		perror("create pthreat error1\n");
	}	
	return thread;	
}

thread_pool thread_pool_create(void)
{
	thread_pool pool;
	pool = (thread_pool)malloc(sizeof(struct thread_pool_s));
	
	if (NULL == pool) {
		perror("malloc pool error1\n");
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
		perror("the pool is empty!\n");
		return -1;
	}
	
	if (NULL == thread) {
		perror("the thread is illegal!\n");
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
		perror("the thead to be deleted is empty!\n");
		return -1;
	}
	list_del(&thread->list);
	free(thread);
	thread = NULL;

	return 0;
}

void *do_something(void *arg) {
    int n = *(int *)arg;
    printf("task #%d started\n", n);
    printf("task #%d finished\n", n);
		return NULL;
}

int main(int argc, char *argv[])
{
	thread_t thread;
	int a = 10;
	thread = thread_create(do_something, (void *)(&a));
	while(1){
	}
	return 0;
}
