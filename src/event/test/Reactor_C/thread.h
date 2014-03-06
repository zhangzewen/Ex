#ifndef _HTTP_THREAD_H_INCLUDED__
#define _HTTP_THREAD_H_INCLUDED__


struct conn_queue_item {
	int	sfd;
	int event_flags;
	int read_buffer_size;
	struct conn_queue_item *next;
};

struct conn_queue｛
	struct conn_queue_item *head;
	struct conn_queue_item *tail;
	pthread_mutex_t lock;	
};


struct libevent_thread{
	pthread_t thread_id;
	struct event_base *base;
	struct event notify_event;
	int notify_receive_fd;
	int notify_send_fd;
	struct conn_queue *new_conn_queue;
};

struct libevent_dispatcher_thread
{
	pthread_t thread_id;
	struct event_base *base;
};

#define hashsize(n) ((unsigned long int)1 << (n))
#define hashmask(n) (hashsize(n) - 1)

void conn_queue_init(struct conn_queue *cq);

struct conn_queue_item *cq_pop(struct conn_queue *cq);

void cp_push(struct conn_queue *cq, struct conn_queue_item *item);

struct conn_queue_item *cqi_new();

void cqi_free(struct conn_queue_item *item);

void create_worker(void *(*func)(void *), void *arg);

void setup_thread(struct libevent_thread *me);

void wait_for_thread_registration(int nthreads);

void register_thread_initialized(void);

void *worker_libevent(void *arg);

void thread_libevent_process(int fd, short which, void *arg);

void dispatch_conn_new(int sfd, enum conn_states init_state, int event_flags,
												int read_buffer_size, enum network_transport transport);

void thread_init(int nthreads, struct event_base *main_base);


#endif






