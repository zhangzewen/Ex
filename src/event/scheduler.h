
#ifndef _HTTP_SCHEDULER_H_INCLUDED
#define _HTTP_SCHEDULER_H_INCLUDED

#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <assert.h>
#include <fcntl.h>
#include <errno.h>

#include <timer.h>

typedef struct _thread 
{
	unsigned long id;
	unsigned char type;		/* thread type */
	struct _thread *next;		/* next pointer of the thread */
	struct _thread *prev;		/* previous pointer of the thread */
	struct _thread_master *master;	/* pointer to the struct thread_master. */
	int (*func)(struct _thread *);	/* event function */
	void *arg;			/* event argument */
	TIMEVAL sands;			/* rest of time sands value. */
	union 
	{
		int val;		/* second argument of the event. */
		int fd;			/* file descriptor in case of read/write. */
		struct 
		{
			pid_t pid;	/* process id a child thread is wanting. */
			int status;	/* return status of the process */
		} c;
	} u;
} thread_t;

/* Linked list of thread. */
typedef struct _thread_list 
{
	thread_t *head;
	thread_t *tail;
	int count;
} thread_list_t;

/* Master of the theads. */
typedef struct _thread_master 
{
	thread_list_t read;
	thread_list_t write;
	thread_list_t timer;
	thread_list_t child;
	thread_list_t event;
	thread_list_t ready;
	thread_list_t unuse;
	fd_set readfd;
	fd_set writefd;
	fd_set exceptfd;
	unsigned long alloc;
} thread_master_t;

#define THREAD_READ		0
#define THREAD_WRITE		1
#define THREAD_TIMER		2
#define THREAD_EVENT		3
#define THREAD_CHILD		4
#define THREAD_READY		5
#define THREAD_UNUSED		6
#define THREAD_WRITE_TIMEOUT	7
#define THREAD_READ_TIMEOUT	8
#define THREAD_CHILD_TIMEOUT	9
#define THREAD_TERMINATE	10
#define THREAD_READY_FD		11

/* MICRO SEC def */
#define BOOTSTRAP_DELAY TIMER_HZ
#define RESPAWN_TIMER	60*TIMER_HZ

/* Macros. */
#define THREAD_ARG(X) ((X)->arg)
#define THREAD_FD(X)  ((X)->u.fd)
#define THREAD_VAL(X) ((X)->u.val)
#define THREAD_CHILD_PID(X) ((X)->u.c.pid)
#define THREAD_CHILD_STATUS(X) ((X)->u.c.status)

/* global vars exported */
extern thread_master_t *master;

/* Prototypes. */
extern thread_master_t *thread_make_master(void);
extern thread_t *thread_add_terminate_event(thread_master_t *);
extern void thread_destroy_master(thread_master_t *);
extern thread_t *thread_add_read(thread_master_t *, int (*func) (thread_t *), void *, int, long);
extern thread_t *thread_add_write(thread_master_t *, int (*func) (thread_t *), void *, int, long);
extern thread_t *thread_add_timer(thread_master_t *, int (*func) (thread_t *), void *, long);
extern thread_t *thread_add_child(thread_master_t *, int (*func) (thread_t *), void *, pid_t, long);
extern thread_t *thread_add_event(thread_master_t *, int (*func) (thread_t *), void *, int);
extern void thread_cancel(thread_t *);
extern void thread_cancel_event(thread_master_t *, void *);
extern thread_t *thread_fetch(thread_master_t *, thread_t *);
extern void thread_child_handler(void *, int);
extern void thread_call(thread_t *);
extern void launch_scheduler(void);

#endif
