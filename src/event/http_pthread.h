#ifndef	__HTTP_PTHREAD_H_INCLUDED
#define	__HTTP_PTHREAD_H_INCLUDED
#include <pthread.h>
#include "list.h"
#include "http_error.h"

#define HTTP_PTHREAD_UNKNOWN   0x000
#define HTTP_PTHREAD_READY			0x001
#define HTTP_PTHREAD_RUNNING		0x002
#define HTTP_PTHREAD_STOP			0x003

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
} *pthread_task;

struct task_queue{
	unsigned int current_tasks;
	unsigned int max_tasks;
	unsigned int increase_step;
	unsigned int limit_task_num;
	struct list_head task_queue_head;
};

pthread_task pthread_task_create(void *arg, void *(*fun)(void *arg));


#endif	
