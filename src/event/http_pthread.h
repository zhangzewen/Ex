#ifndef	__HTTP_PTHREAD_H_INCLUDED
#define	__HTTP_PTHREAD_H_INCLUDED
#include <pthread.h>
#include "list.h"
#include "http_error.h"
#define HTTP_PTHREAD_UNKNOWN   0x000
#define HTTP_PTHREAD_READY			0x001
#define HTTP_PTHREAD_RUNNING		0x002
#define HTTP_PTHREAD_STOP			0x003
/*************************************************************
*
* task_queu
*-----task_queue  
*         \------pthread_task_t
*														\-----pthread_task_t	
*																					\-----.......
*
*
*
*
*thread_pool:
*---------pthread_pool
*              \-------thread_t
*                         \------------thread_t
*																					\----------.....
*
*******************************************************/
typedef struct thread_s{
	struct list_head list;
	pthread_t *pthread_id;
} *thread_t;

struct pthread_pool{
	unsigned int current_pthreads;
	unsigned int max_pthreds;
	unsigned int increase_step;
	struct list_head threads_head;	
};

typedef struct pthread_task_t{
	pthread_t *pthread_id;
	int status;
	void *arg;
	void *(*task_func)(void *arg);
	struct list_head list;
}*pthread_task;

struct task_queue{
	unsigned int current_tasks;
	unsigned int max_tasks;
	unsigned int increase_step;
	struct list_head task_queue_head;
};

void *start_routine(void *arg);

void master_work();//master process
void pthread_work();//work pthread
void work_state();

void work_pause();//work --->pause
void work_start();//work-->start
void work_stop();//work--->stop
void work_init();//work--->init
void work_destory();//destory work pthread
void work_run();//run pthread

void destroy_list(struct list_head *head);

pthread_task pthread_task_create(void);

pthread_pool pool_create(int num);

void add_task(void *(*task_func)(void *), void *arg ); //添加任务
#endif	
