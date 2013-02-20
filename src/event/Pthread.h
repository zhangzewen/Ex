#ifndef	__HTTP_PTHREAD_H_INCLUDED
#define	__HTTP_PTHREAD_H_INCLUDED
#include <pthread.h>

struct pthread_pool_t{
	pthread_mutex_t pool_mutex;
	pthread_mutex_t queue_mutex;
	pthread_cond_t queue_cond_ready;
	unsigned int current_pthreads;
	unsigned int limit_pthread_num;
	struct list_head pthread_head;	
	struct list_head wait_pthread_head;
};
typedef struct pthread_pool_t *pthread_pool;
struct pthread_head_t{
	int pthread_id;
	int status;
	void *arg;
	void *(*task_func)(void *arg);
};

void	Pthread_create(pthread_t *, const pthread_attr_t *,
					   void * (*)(void *), void *);
void	Pthread_join(pthread_t, void **);
void	Pthread_detach(pthread_t);
void	Pthread_kill(pthread_t, int);

void	Pthread_mutexattr_init(pthread_mutexattr_t *);
void	Pthread_mutexattr_setpshared(pthread_mutexattr_t *, int);
void	Pthread_mutex_init(pthread_mutex_t *, pthread_mutexattr_t *);
void	Pthread_mutex_lock(pthread_mutex_t *);
void	Pthread_mutex_unlock(pthread_mutex_t *);

void	Pthread_cond_broadcast(pthread_cond_t *);
void	Pthread_cond_signal(pthread_cond_t *);
void	Pthread_cond_wait(pthread_cond_t *, pthread_mutex_t *);
void	Pthread_cond_timedwait(pthread_cond_t *, pthread_mutex_t *,
							   const struct timespec *);

void	Pthread_key_create(pthread_key_t *, void (*)(void *));
void	Pthread_setspecific(pthread_key_t, const void *);
void	Pthread_once(pthread_once_t *, void (*)(void));

void master_work();//master process
void pthread_work();//work pthread
void work_state();

void work_pause();//work --->pause
void work_start();//work-->start
void work_stop();//work--->stop
void work_init();//work--->init
void work_destory();//destory work pthread
void work_run();//run pthread
void add_task(); //添加任务
pthread_pool pool_create(int num)
{
	pthread_pool new_pool;
	new_pool = (struct pthread_pool_t *)malloc(sizeof(struct pthread_pool_t));
	if (NULL == new_pool)
		error_quit("can not create a pthread pool !\n");
	new_pool->limit_pthread_num = num;
	new_pool->current_pthreads = 0;
	pthread_mutex_init(&new_pool->pool_mutex);	
	pthread_mutex_init(&new_pool->queue_mutex);	
	pthread_cond_init(&new_pool->queue_cond_ready);
}

#endif	
