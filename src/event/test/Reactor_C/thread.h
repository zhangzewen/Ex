#ifndef _HTTP_THREAD_H_INCLUDED__
#define _HTTP_THREAD_H_INCLUDED__


enum conn_states{
	conn_listening,
	conn_new_cmd,
	conn_waiting,
	conn_read,
	conn_parse_cmd,
	conn_write,
	conn_nread,
	conn_swallow,
	conn_mwrite,
	conn_max_state
};

enum network_transport{
	local_transport,
	tcp_transport,
	udp_transport
};

struct conn_queue_item {
	int	sfd;
	enum conn_states init_state;
	int event_flags;
	int read_buffer_size;
	enum network_tranport transport;
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
	struct thread_stats stats;
	struct conn_queue *new_conn_queue;
};

struct libevent_dispatcher_thread
{
	pthread_t thread_id;
	struct event_base *base;
};



pthread_mutex_t conn_lock = PTHREAD_MUTEX_INITIALIZER;


	
#endif
