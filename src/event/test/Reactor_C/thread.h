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


#endif
