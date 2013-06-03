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

typedef struct thread_st *thread_t;
typedef struct thread_pool_st *thread_pool;

typedef struct thread_task_st *thread_task_t;
typedef struct task_queue_st *task_queue_t;

struct thread_st{
	struct list_head list;
	pthread_t *pid;
	int num;
};


/*
*thread_pool_mutex and thread_pool_ready
*both for current_threads
*threads_mutex just for the threas
*/
struct thread_pool_s{
	pthread_mutex_t thread_pool_mutex;
	pthread_cond_t thread_pool_ready;
	unsigned int current_threads;
	unsigned int max_threads;
	struct list_head threads_head;	
};




/*************task queue *******************/
struct thread_task_s{
	unsigned int task_id;
	int status;
	void *arg; //arg -------->private_data_st
	void *(*task_func)(void *arg);
	struct list_head list;
};

struct task_queue_s{
	pthread_mutex_t task_queue_mutex;
	pthread_cond_t task_queue_ready;
	unsigned int current_tasks;
	unsigned int max_tasks;
	struct list_head task_queue_head;
};






/**********pool manage*****************************/


thread_pool thread_pool_create(void);
thread_t thread_create(const pthread_attr_t *attr, void *(*start_routine)(void *arg), void *arg);
int add_thread(thread_pool pool, thread_t thread);

int destory_thread(thread_t thead);
int get_current_threads_count(thread_pool pool);
int destory_threads_pool(thread_pool pool);


/***********task manage******************************/
thread_task thread_task_create(void *(*fun)(void *arg), unsigned int num);

task_queue task_queue_create(void);

int add_task(task_queue queue, thread_task task);
void *task_fun(void *arg);



int destory_task(thread_task task);

int get_current_tasks_count(task_queue queue);
void *start_routine(void *arg);
#endif	
