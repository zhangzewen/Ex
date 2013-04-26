#ifndef	__HTTP_PTHREAD_H_INCLUDED
#define	__HTTP_PTHREAD_H_INCLUDED
#include <pthread.h>
#include "list.h"
#include "http_error.h"

#define HTTP_PTHREAD_UNKNOWN   0x000
#define HTTP_PTHREAD_READY			0x001
#define HTTP_PTHREAD_RUNNING		0x002
#define HTTP_PTHREAD_STOP			0x003
/***************thread pool***************/
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

thread_pool thread_pool_create(void);
thread_t thread_create(const pthread_attr_t *attr, void *(*start_routine)(void *arg), void *arg);
int add_thread(thread_pool pool, thread_t thread);

int destory_thread(thread_t thead);
int get_current_threads_count(thread_pool pool);
int destory_threads_pool(thread_pool pool);
/*************task queue *******************/
typedef struct thread_task_s{
	pthread_t *thread_id;
	int status;
	void *arg;
	void *(*task_func)(void *arg);
	struct list_head list;
} *thread_task;

typedef struct task_queue_s{
	pthread_mutex_t task_queue_mutex;
	pthread_cond_t task_queue_cond;
	unsigned int current_tasks;
	unsigned int max_tasks;
	unsigned int increase_step;
	unsigned int limit_task_num;
	struct list_head task_queue_head;
}*task_queue;

thread_task thread_task_create(void *arg, void *(*fun)(void *arg));

task_queue task_queue_create(void);

int add_task(task_queue queue, thread_task task);



int destory_task(thread_task task);

int get_current_tasks_count(task_queue queue);
void *start_routine(void *arg);
#endif	
