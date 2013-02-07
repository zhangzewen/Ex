
#include <signal.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <unistd.h>
#include "scheduler.h"
#include "http_signal.h"
#include "http_timer.h"
#include <syslog.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* global vars */
thread_master_t *master = NULL;

/* Make thread master. */
thread_master_t *thread_make_master(void)
{
	thread_master_t *new;

	new = (thread_master_t *)malloc(sizeof(thread_master_t));
	return new;
}

/* Add a new thread to the list. */
static void thread_list_add(thread_list_t *list, thread_t *thread)
{
	thread->next = NULL;
	thread->prev = list->tail;
	if (list->tail)
		list->tail->next = thread;
	else
		list->head = thread;
	list->tail = thread;
	list->count++;
}

/* Add a new thread to the list. */
void thread_list_add_before(thread_list_t *list, thread_t *point, thread_t *thread)
{
	thread->next = point;
	thread->prev = point->prev;
	if (point->prev)
		point->prev->next = thread;
	else
		list->head = thread;
	point->prev = thread;
	list->count++;
}

/* Add a thread in the list sorted by timeval */
void thread_list_add_timeval(thread_list_t *list, thread_t *thread)
{
	thread_t *tt;

	for (tt = list->head; tt; tt = tt->next)
	{
		if (timer_cmp(thread->sands, tt->sands) <= 0)
			break;
	}

	if (tt)
		thread_list_add_before(list, tt, thread);
	else
		thread_list_add(list, thread);
}

/* Delete a thread from the list. */
thread_t *thread_list_delete(thread_list_t *list, thread_t *thread)
{
	if (thread->next)
		thread->next->prev = thread->prev;
	else
		list->tail = thread->prev;
	if (thread->prev)
		thread->prev->next = thread->next;
	else
		list->head = thread->next;
	thread->next = thread->prev = NULL;
	list->count--;
	return thread;
}

/* Free all unused thread. */
static void thread_clean_unuse(thread_master_t *m)
{
	thread_t *thread;

	thread = m->unuse.head;
	while (thread)
	{
		thread_t *t;

		t = thread;
		thread = t->next;

		thread_list_delete(&m->unuse, t);

		/* free the thread */
		free(t);
		m->alloc--;
	}
}

/* Move thread to unuse list. */
static void thread_add_unuse(thread_master_t *m, thread_t *thread)
{
	assert(m != NULL);
	assert(thread->next == NULL);
	assert(thread->prev == NULL);
	assert(thread->type == THREAD_UNUSED);
	thread_list_add(&m->unuse, thread);
}

/* Move list element to unuse queue */
static void thread_destroy_list(thread_master_t *m, thread_list_t thread_list)
{
	thread_t *thread;

	thread = thread_list.head;

	while (thread)
	{
		thread_t *t;

		t = thread;
		thread = t->next;

		if (t->type == THREAD_READY_FD ||
		    t->type == THREAD_READ ||
		    t->type == THREAD_WRITE ||
		    t->type == THREAD_READ_TIMEOUT ||
		    t->type == THREAD_WRITE_TIMEOUT)
			close(t->u.fd);

		thread_list_delete(&thread_list, t);
		t->type = THREAD_UNUSED;
		thread_add_unuse(m, t);
	}
}

/* Cleanup master */
static void thread_cleanup_master(thread_master_t *m)
{
	/* Unuse current thread lists */
	thread_destroy_list(m, m->read);
	thread_destroy_list(m, m->write);
	thread_destroy_list(m, m->timer);
	thread_destroy_list(m, m->event);
	thread_destroy_list(m, m->ready);

	/* Clear all FDs */
	FD_ZERO(&m->readfd);
	FD_ZERO(&m->writefd);
	FD_ZERO(&m->exceptfd);

	/* Clean garbage */
	thread_clean_unuse(m);
}

/* Stop thread scheduler. */
void thread_destroy_master(thread_master_t *m)
{
	thread_cleanup_master(m);
	free(m);
}

/* Delete top of the list and return it. */
thread_t *thread_trim_head(thread_list_t *list)
{
	if (list->head)
		return thread_list_delete(list, list->head);
	return NULL;
}

/* Make new thread. */
thread_t *thread_new(thread_master_t *m)
{
	thread_t *new;

	/* If one thread is already allocated return it */
	if (m->unuse.head)
	{
		new = thread_trim_head(&m->unuse);
		memset(new, 0, sizeof(thread_t));
		return new;
	}

	new = (thread_t *)malloc(sizeof(thread_t));
	m->alloc++;
	return new;
}

/* Add new read thread. */
thread_t *thread_add_read(thread_master_t *m, int (*func)(thread_t *), void *arg, int fd, long timer)
{
	thread_t *thread;

	assert(m != NULL);

	if (FD_ISSET(fd, &m->readfd))
	{
		syslog(LOG_INFO, "There is already read fd [%d]", fd);
		return NULL;
	}

	thread = thread_new(m);
	thread->type = THREAD_READ;
	thread->id = 0;
	thread->master = m;
	thread->func = func;
	thread->arg = arg;
	FD_SET(fd, &m->readfd);
	thread->u.fd = fd;

	/* Compute read timeout value */
	set_time_now();
	thread->sands = timer_add_long(time_now, timer);

	/* Sort the thread. */
	thread_list_add_timeval(&m->read, thread);

	return thread;
}

/** 
Add new write thread. 
func:  tcp_check_thread
timer: tcp_check->connection_to
**/
thread_t *thread_add_write(thread_master_t *m, int (*func)(thread_t *), void *arg, int fd, long timer)
{
	thread_t *thread;

	assert(m != NULL);

	if (FD_ISSET(fd, &m->writefd)) {
		syslog(LOG_INFO, "There is already write fd [%d]", fd);
		return NULL;
	}

	thread = thread_new(m);
	thread->type = THREAD_WRITE;		/* write type */
	thread->id = 0;
	thread->master = m;
	thread->func = func;
	thread->arg = arg;
	FD_SET(fd, &m->writefd);		/* add fd to write fd set */
	thread->u.fd = fd;

	/* Compute write timeout value */
	set_time_now();
	thread->sands = timer_add_long(time_now, timer);

	/* Sort the thread. */
	thread_list_add_timeval(&m->write, thread);

	return thread;
}

/* Add timer event thread. */
thread_t *thread_add_timer(thread_master_t *m, int (*func)(thread_t *), void *arg, long timer)
{
	thread_t *thread;

	assert(m != NULL);

	thread = thread_new(m);			/* create new thread */
	thread->type = THREAD_TIMER;
	thread->id = 0;
	thread->master = m;
	thread->func = func;
	thread->arg = arg;			/* point to general checker */

	/* Do we need jitter here? */
	set_time_now();
	thread->sands = timer_add_long(time_now, timer);

	/* Sort by timeval. */
	thread_list_add_timeval(&m->timer, thread);

	return thread;
}

/* Add a child thread. */
thread_t *thread_add_child(thread_master_t *m, int (*func)(thread_t *), void *arg, pid_t pid, long timer)
{
	thread_t *thread;

	assert(m != NULL);

	thread = thread_new(m);
	thread->type = THREAD_CHILD;
	thread->id = 0;
	thread->master = m;
	thread->func = func;
	thread->arg = arg;
	thread->u.c.pid = pid;
	thread->u.c.status = 0;

	/* Compute write timeout value */
	set_time_now();
	thread->sands = timer_add_long(time_now, timer);

	/* Sort by timeval. */
	thread_list_add_timeval(&m->child, thread);

	return thread;
}

/* Add simple event thread. */
thread_t *thread_add_event(thread_master_t *m, int (*func)(thread_t *), void *arg, int val)
{
	thread_t *thread;

	assert(m != NULL);

	thread = thread_new(m);
	thread->type = THREAD_EVENT;
	thread->id = 0;
	thread->master = m;
	thread->func = func;
	thread->arg = arg;
	thread->u.val = val;
	thread_list_add(&m->event, thread);

	return thread;
}

/* Add simple event thread. */
thread_t *thread_add_terminate_event(thread_master_t *m)
{
	thread_t *thread;

	assert(m != NULL);

	thread = thread_new(m);
	thread->type = THREAD_TERMINATE;
	thread->id = 0;
	thread->master = m;
	thread->func = NULL;
	thread->arg = NULL;
	thread->u.val = 0;
	thread_list_add(&m->event, thread);

	return thread;
}

/* Cancel thread from scheduler. */
void thread_cancel(thread_t *thread)
{
	switch (thread->type)
	{
		case THREAD_READ:
			assert(FD_ISSET(thread->u.fd, &thread->master->readfd));
			FD_CLR(thread->u.fd, &thread->master->readfd);
			thread_list_delete(&thread->master->read, thread);
			break;
		case THREAD_WRITE:
			assert(FD_ISSET
			       (thread->u.fd, &thread->master->writefd));
			FD_CLR(thread->u.fd, &thread->master->writefd);
			thread_list_delete(&thread->master->write, thread);
			break;
		case THREAD_TIMER:
			thread_list_delete(&thread->master->timer, thread);
			break;
		case THREAD_CHILD:
			/* Does this need to kill the child, or is that the
			 * caller's job?
			 * This function is currently unused, so leave it for now.
			 */
			thread_list_delete(&thread->master->child, thread);
			break;
		case THREAD_EVENT:
			thread_list_delete(&thread->master->event, thread);
			break;
		case THREAD_READY:
		case THREAD_READY_FD:
			thread_list_delete(&thread->master->ready, thread);
			break;
		default:
			break;
	}

	thread->type = THREAD_UNUSED;
	thread_add_unuse(thread->master, thread);
}

/* Delete all events which has argument value arg. */
void thread_cancel_event(thread_master_t *m, void *arg)
{
	thread_t *thread;

	thread = m->event.head;
	while (thread)
	{
		thread_t *t;

		t = thread;
		thread = t->next;

		if (t->arg == arg)
		{
			thread_list_delete(&m->event, t);
			t->type = THREAD_UNUSED;
			thread_add_unuse(m, t);
		}
	}
}

/* Update timer value */
static void thread_update_timer(thread_list_t *list, struct timeval *timer_min)
{
	if (list->head)
	{
		if (!timer_isnull(*timer_min))
		{
			if (timer_cmp(list->head->sands, *timer_min) <= 0)
			{
				*timer_min = list->head->sands;
			}
		}
		else
		{
			*timer_min = list->head->sands;
		}
	}
}

/* Compute the wait timer. Take care of timeouted fd */
static void thread_compute_timer(thread_master_t *m, struct timeval *timer_wait)
{
	struct timeval timer_min;

	/* Prepare timer */
	timer_reset(&timer_min);
	thread_update_timer(&m->timer, &timer_min);
	thread_update_timer(&m->write, &timer_min);
	thread_update_timer(&m->read, &timer_min);
	thread_update_timer(&m->child, &timer_min);

	/* Take care about monothonic clock */
	if (!timer_isnull(timer_min))
	{
		timer_min = timer_sub(timer_min, time_now);
		if (timer_min.tv_sec < 0)
		{
			timer_min.tv_sec = timer_min.tv_usec = 0;
		}
		else if (timer_min.tv_sec >= 1)
		{
			timer_min.tv_sec = 1;
			timer_min.tv_usec = 0;
		}

		timer_wait->tv_sec = timer_min.tv_sec;
		timer_wait->tv_usec = timer_min.tv_usec;
	}
	else
	{
		timer_wait->tv_sec = 1;
		timer_wait->tv_usec = 0;
	}
}

/* Fetch next ready thread. */
thread_t *thread_fetch(thread_master_t *m, thread_t *fetch)
{
	int ret, old_errno;
	thread_t *thread;
	fd_set readfd;
	fd_set writefd;
	fd_set exceptfd;
	struct timeval timer_wait;
	int signal_fd;

	assert(m != NULL);

	/* Timer initialization */
	memset(&timer_wait, 0, sizeof(struct timeval));

retry:	/* When thread can't fetch try to find next thread again. */

	/* If there is event process it first. */
	while ((thread = thread_trim_head(&m->event)))
	{
		*fetch = *thread;						/* structure assignment */

		/* If daemon hanging event is received return NULL pointer */
		if (thread->type == THREAD_TERMINATE)
		{
			thread->type = THREAD_UNUSED;
			thread_add_unuse(m, thread);
			return NULL;
		}
		thread->type = THREAD_UNUSED;
		thread_add_unuse(m, thread);
		return fetch;
	}

	/* If there is ready threads process them */
	while ((thread = thread_trim_head(&m->ready)))				/* event handle entrance */
	{
		*fetch = *thread;						/* structure assignment */
		thread->type = THREAD_UNUSED;
		thread_add_unuse(m, thread);
		return fetch;
	}

	/*
	 * Re-read the current time to get the maximum accuracy.
	 * Calculate select wait timer. Take care of timeouted fd.
	 */
	set_time_now();
	thread_compute_timer(m, &timer_wait);

	/* Call select function. */
	readfd = m->readfd;
	writefd = m->writefd;
	exceptfd = m->exceptfd;

	signal_fd = signal_rfd();		/* return (signal_pipe[0]); */
	FD_SET(signal_fd, &readfd);

	ret = select(FD_SETSIZE, &readfd, &writefd, &exceptfd, &timer_wait);

	/* we have to save errno here because the next syscalls will set it */
	old_errno = errno;

	/* handle signals synchronously, including child reaping */
	if (FD_ISSET(signal_fd, &readfd))
		signal_run_callback();

	/* Update current time */
	set_time_now();

	if (ret < 0)
	{
		if (old_errno == EINTR)
			goto retry;
		/* Real error. */
		syslog(LOG_INFO, "select error: %s", strerror(old_errno));
		assert(0);
	}

	/* Timeout children */
	thread = m->child.head;
	while (thread)
	{
		thread_t *t;

		t = thread;
		thread = t->next;

		if (timer_cmp(time_now, t->sands) >= 0)
		{
			thread_list_delete(&m->child, t);		/* delete from child list */
			thread_list_add(&m->ready, t);			/* append to ready list */
			t->type = THREAD_CHILD_TIMEOUT;
		}
	}

	/* Read thead. */
	thread = m->read.head;
	while (thread)
	{
		thread_t *t;

		t = thread;
		thread = t->next;

		if (FD_ISSET(t->u.fd, &readfd))
		{
			assert(FD_ISSET(t->u.fd, &m->readfd));
			FD_CLR(t->u.fd, &m->readfd);
			thread_list_delete(&m->read, t);		/* delete from read list */
			thread_list_add(&m->ready, t);			/* append to ready list */
			t->type = THREAD_READY_FD;			/* convert thread type */
		}
		else
		{
			if (timer_cmp(time_now, t->sands) >= 0)		/* (a, b), return 1 if a > b */
			{
				FD_CLR(t->u.fd, &m->readfd);
				thread_list_delete(&m->read, t);
				thread_list_add(&m->ready, t);
				t->type = THREAD_READ_TIMEOUT;		/* convert thread type */
			}
		}
	}

	/* Write thead. */
	thread = m->write.head;
	while (thread)
	{
		thread_t *t;

		t = thread;
		thread = t->next;

		if (FD_ISSET(t->u.fd, &writefd))
		{
			assert(FD_ISSET(t->u.fd, &writefd));
			FD_CLR(t->u.fd, &m->writefd);
			thread_list_delete(&m->write, t);		/* delete from write list */
			thread_list_add(&m->ready, t);			/* append to ready list */
			t->type = THREAD_READY_FD;			/* convert to ready type */
		}
		else
		{
			if (timer_cmp(time_now, t->sands) >= 0)		/* timeout */
			{
				FD_CLR(t->u.fd, &m->writefd);
				thread_list_delete(&m->write, t);
				thread_list_add(&m->ready, t);
				t->type = THREAD_WRITE_TIMEOUT;		/* convert thread type */
			}
		}
	}
	/* Exception thead. */
	/*... */

	/* Timer update. */
	thread = m->timer.head;
	while (thread)
	{
		thread_t *t;

		t = thread;
		thread = t->next;

		if (timer_cmp(time_now, t->sands) >= 0)			/* (a, b) return 1 if a > b */
		{
			thread_list_delete(&m->timer, t);		/* detach from timer queue */
			thread_list_add(&m->ready, t);			/* attach to ready queue */
			t->type = THREAD_READY;				/* convert thread type */
		}
	}

	/* Return one event. */
	thread = thread_trim_head(&m->ready);

	/* There is no ready thread. */
	if (!thread)
		goto retry;

	*fetch = *thread;						/* structure assignment */
	thread->type = THREAD_UNUSED;
	thread_add_unuse(m, thread);

	return fetch;
}

/* Synchronous signal handler to reap child processes */
void thread_child_handler(void *v, int sig)
{
	thread_master_t *m = v;

	/*
	 * This is O(n^2), but there will only be a few entries on
	 * this list.
	 */
	thread_t *thread;
	pid_t pid;
	int status = 77;
	while ((pid = waitpid(-1, &status, WNOHANG)))
	{
		if (pid == -1)
		{
			if (errno == ECHILD)
				return;
			printf("waitpid error: %s", strerror(errno));
			assert(0);
		}
		else
		{
			thread = m->child.head;
			while (thread)
			{
				thread_t *t;
				t = thread;
				thread = t->next;
				if (pid == t->u.c.pid)
				{
					thread_list_delete(&m->child, t);
					thread_list_add(&m->ready, t);
					t->u.c.status = status;
					t->type = THREAD_READY;
					break;
				}
			}
		}
	}
}

/* Make unique thread id for non pthread version of thread manager. */
unsigned long int thread_get_id(void)
{
	static unsigned long int counter = 0;
	return ++counter;
}

/* Call thread ! */
void thread_call(thread_t *thread)
{
	thread->id = thread_get_id();
	(*thread->func)(thread);
}

/* Our infinite scheduling loop */
void launch_scheduler(void)
{
	thread_t thread;

	signal_set(SIGCHLD, thread_child_handler, master);

	/*
	 * Processing the master thread queues,
	 * return and execute one ready thread.
	 */
	while (thread_fetch(master, &thread))
	{
		/* Run until error, used for debuging only */
#ifdef _DEBUG_
		if ((debug & 520) == 520)
		{
			debug &= ~520;
			thread_add_terminate_event(master);
		}
#endif
		thread_call(&thread);
	}
}
