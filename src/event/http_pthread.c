#include "http_pthread.h"
#include <errno.h>
#include "http_error.h"
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
void Pthread_create(pthread_t *tid, const pthread_attr_t *attr,
			   void * (*func)(void *), void *arg)
{
	int		n;

	if ( (n = pthread_create(tid, attr, func, arg)) == 0)
		return;
	errno = n;
}

void Pthread_join(pthread_t tid, void **status)
{
	int		n;

	if ( (n = pthread_join(tid, status)) == 0)
		return;
	errno = n;
}

void Pthread_detach(pthread_t tid)
{
	int		n;

	if ( (n = pthread_detach(tid)) == 0)
		return;
	errno = n;
}

void Pthread_kill(pthread_t tid, int signo)
{
	int		n;

	if ( (n = pthread_kill(tid, signo)) == 0)
		return;
	errno = n;
}

void Pthread_mutexattr_init(pthread_mutexattr_t *attr)
{
	int		n;

	if ( (n = pthread_mutexattr_init(attr)) == 0)
		return;
	errno = n;
}

#ifdef	_POSIX_THREAD_PROCESS_SHARED
void Pthread_mutexattr_setpshared(pthread_mutexattr_t *attr, int flag)
{
	int		n;

	if ( (n = pthread_mutexattr_setpshared(attr, flag)) == 0)
		return;
	errno = n;
}
#endif

void Pthread_mutex_init(pthread_mutex_t *mptr, pthread_mutexattr_t *attr)
{
	int		n;

	if ( (n = pthread_mutex_init(mptr, attr)) == 0)
		return;
	errno = n;
}

/* include Pthread_mutex_lock */
void Pthread_mutex_lock(pthread_mutex_t *mptr)
{
	int		n;

	if ( (n = pthread_mutex_lock(mptr)) == 0)
		return;
	errno = n;
}
/* end Pthread_mutex_lock */

void Pthread_mutex_unlock(pthread_mutex_t *mptr)
{
	int		n;

	if ( (n = pthread_mutex_unlock(mptr)) == 0)
		return;
	errno = n;
}

void Pthread_cond_broadcast(pthread_cond_t *cptr)
{
	int		n;

	if ( (n = pthread_cond_broadcast(cptr)) == 0)
		return;
	errno = n;
}

void Pthread_cond_signal(pthread_cond_t *cptr)
{
	int		n;

	if ( (n = pthread_cond_signal(cptr)) == 0)
		return;
	errno = n;
}

void Pthread_cond_wait(pthread_cond_t *cptr, pthread_mutex_t *mptr)
{
	int		n;

	if ( (n = pthread_cond_wait(cptr, mptr)) == 0)
		return;
	errno = n;
}

void Pthread_cond_timedwait(pthread_cond_t *cptr, pthread_mutex_t *mptr,
					   const struct timespec *tsptr)
{
	int		n;

	if ( (n = pthread_cond_timedwait(cptr, mptr, tsptr)) == 0)
		return;
	errno = n;
}

void Pthread_once(pthread_once_t *ptr, void (*func)(void))
{
	int		n;

	if ( (n = pthread_once(ptr, func)) == 0)
		return;
	errno = n;
}

void Pthread_key_create(pthread_key_t *key, void (*func)(void *))
{
	int		n;

	if ( (n = pthread_key_create(key, func)) == 0)
		return;
	errno = n;
}

void Pthread_setspecific(pthread_key_t key, const void *value)
{
	int		n;

	if ( (n = pthread_setspecific(key, value)) == 0)
		return;
	errno = n;
}


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
	Pthread_mutex_init(&new_pool->pool_mutex, NULL);	
	Pthread_mutex_init(&new_pool->queue_mutex, NULL);	
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
	Pthread_create(new->pthread_id, NULL,start_routine, new->arg);	
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

