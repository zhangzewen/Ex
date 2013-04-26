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
	struct list_head threads_head;	
}*thread_pool;


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
