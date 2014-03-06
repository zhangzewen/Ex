#include "thread.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


static void thread_libevent_process(int fd, short which, void *arg);
static struct conn_queue_item *cqi_freelist;
static struct pthread_mutex_t cqi_freelist_lock;
static pthread_mutex_t *item_locks;

#define hashsize(n) ((unsigned long int)1 << (n))
#define hashmask(n) (hashsize(n) - 1)

static struct libevent_thread *threads;

static int init_count = 0;
static pthread_mutex_t init_lock;
static pthread_cond_t init_cond;

static libevent_dispatcher_thread dispatcher_thread; //主线程

static void conn_queue_init(struct conn_queue *cq)
{
	pthread_mutex_init(&cq->lock, NULL);
	cp->head = NULL;
	cp->tail = NULL;
}

//从头部开始pop
static struct conn_queue_item *cq_pop(struct conn_queue *cq)
{
	struct conn_queue_item *item;
	pthread_mutex_lock(&cq->lock);

	item = cq->head;
	
	if (NULL != item) {
		cq->head = item->next;
		if (NULL == cq->head)
			cp->tail = NULL;
	}

	pthread_mutex_unlock(&cp->lock);

	return item;
}

//从尾部push
static void cp_push(struct conn_queue *cq, struct conn_queue_item *item) {
	item->next = NULL;
	pthread_mutex_lock(&cq->lock);
	if (NULL == cp->tail)
		cq->head = item;
	else
		cp->tail->next = item;
	
	cp->tail = item;
	
	pthread_mutex_unlock(&cq->lock);
}	


static struct conn_queue_item *cqi_new()
{
	struct conn_queue_item *item = NULL;
	pthread_mutex_lock(&cqi_freelist_lock);
	if (cqi_freelist) {
		item = cqi_freelist;
		cqi_freelist = item->next;
	}

	pthread_mutex_unlock(&cqi_freelist_lock);

	if (NULL == item) {
		item = malloc(sizeof(struct conn_queue_item) * ITEMS_PER_ALLOC);
	
		if (NULL == item) {
			STATS_LOCK();
			stats.malloc_fails++;
			STATS_UNLOCK();
			return NULL;
		}


		for (i = 2; i < ITEMS_PER_ALLOC; i++)
			item[i - 1].next = &item[i];


		pthread_mutex_lock(&cqi_freelist_lock);
		item[ITEMS_PER_ALLOC - 1].next = cqi_freelist;

		cqi_freelist = &item[1];
		pthread_mutex_unlock(&cqi_freelist_lock);
	}
	
	return item;
}


static void cqi_free(struct conn_queue_item *item)
{
	pthread_mutex_lock(&cqi_freelist_lock);
	item->next = cqi_freelist;
	cqi_freelist = item;
	pthread_mutex_unlock(&cqi_freelist_lock);
}


static void create_worker(void *(*func)(void *), void *arg)
{
	pthread_t thread;
	pthread_attr_t attr;
	int ret;
	pthread_attr_init(&attr);
	if ((ret = pthread_create(&thread, &attr, func, arg)) != 0) {
		fprintf(stderr, "Can't create thread : %s\n", strerror(ret));
		exit(1);
	}
}


static void setup_thread(libevent_thread *me)
{
	me->base = event_init();

	if (!me->base) {
		fprintf(stderr, "can not alloc event_base");
		exit(1);
	}

	event_set(&me->notify_event, me->notify_receive_fd,
						EV_READ | EV_PRESIST, thread_libevent_process, me);


	event_base_set(me->base, &me->notify_event);

	if (event_add(&me->notify_event, 0) == -1) {
		fprintf(stderr, "can not monitor libevent notify pipe \n");
		exit(1);
	}

	me->new_conn_queue = malloc(sizeof(struct conn_queue));

	if (NULL == me->new_conn_queue) {
		perror("Failed to allocate memory for connection queue");
		exit(EXIT_FAILURE);
	}

	cq_init(me->new_conn_queue);

	if (pthread_mutex_init(&me->stats.mutex, NULL) != 0)
	{
		perror("Failed to initialize mutex");
		exit(EXIT_FAILURE);
	}
}

static void wait_for_thread_registration(int nthreads)
{
	while(init_count < nthreads) {
		pthread_cond_wait(&init_cond, &init_lock);
	}
}


static void register_thread_initialized(void)
{
	pthrread_mutex_lock(&init_lock);
	init_count++;
	pthread_cond_signal(&init_cond);
	pthread_mutex_unlock(&init_lock);
}


static void *worker_libevent(void *arg)
{
	libevent_thread *me = arg;

	me->item_lock_type = ITEM_LOCK_GRANULAR;

	pthread_setspecific(item_lock_type_key, &me->item_lock_type);

	register_thread_initialized();

	event_base_loop(me->base, 0);
	return NULL;
}

static void thread_libevent_process(int fd, short which, void *arg)
{
	libevent_thread *me = arg;
	struct conn_queue *item;
	char buf[1];

	if (read(fd, buf, 1) != 1) {
		if (settings.verbose > 0) {
			fprintf(stderr, "can not read from libevent pipe\n");
		}
	}

	switch(buf[0]) {
		case 'c':
			item = cq_pop(me->new_conn_queue);
	
			if (NULL != item) {
				conn *c = conn_new(item->fd, item->init_stat, item->event_flags,
														item->read_buffer_size, item->transport, me->base);

				if (c == NULL) {
					if (IS_UDP(item->transport)) {
						fprintf(stderr, "can not listen for events no UDP socket \n");
						exit(1);
					}else {
						if (settings.verbose > 0) {
							fprintf(stderr, "can not listen for events on fd %d\n", item->sfd);
						}

						close(item->sfd);
					}
				}else {
					c->thread = me;
				}

				cqi_free(item);
			}
			break;

		case 'l':
			me->item_lock_type = ITEM_LOCK_GRANULAR;
			register_thread_initialized();
			break;

		case 'g':
			me->item_lock_type = ITEM_LOCK_GLOBAL;
			register_thread_initialized();
			break;

	}
	
}


void dispatch_conn_new(int sfd, enum conn_states init_state, int event_flags,
												int read_buffer_size, enum network_transport transport) 
{
	struct conn_queue_item *item = cqi_new();

	char buf[1];

	if (item == NULL) {
		close(sfd);
		fprintf(stderr, "Failed to allocate memory for connection object\n");
		return ;
	}

	int tid = (last_thread + 1) % settings.num_threads;

	libevent_thread *thread = threads +tid;

	last_thread = tid;

	item->sfd = sfd;
	item->init_state = init_state;
	item->event_flags = event_flags;
	item->read_buffer_size = read_buffer_size;
	item->transport = transport;


	cq_push(thread->new_conn_queue, item);

	MEMCACHED_CONN_DISPATCH(sfd, thread->thread_id);

	buf[0] = 'c';

	if (write(thread->nofify_send_fd, buf, 1) != 1) {
		perror("writing to thread notify pipe");
	}		
}


void thread_init(int nthreads, struct event_base *main_base)
{
	int i;
	int power;
	pthread_mutex_init(&cache_lock, NULL);
	pthread_mutex_init(&stats_lock, NULL);
	pthread_mutex_init(&init_lock, NULL);
	pthread_mutex_init(&init_cond, NULL);
	pthread_mutex_init(&cqi_freelist_lock, NULL);

	cqi_freelist = NULL;

	if (nthreads < 3) 
		power = 10;
	else if(nthreads < 4)
		power = 11;
	else if (nthreads < 5)
		power = 12;
	else
		power = 13;


	item_lock_count = hashsize(power);

	item_locks = calloc(item_lock_count, sizeof(pthread_mutex_t));

	if (! item_locks) {
		perror("can not allocate item locks");
		exit(1);
	}

	for (i= 0； i < item_lock_count; i++) {
		pthread_mutex_init(&item_lock[i], NULL);
	}

	pthread_key_create(&item_lock_type_key, NULL);

	pthread_mutex_init(&item_global_lock, NULL);

	threads = calloc(nthreads, sizeof(struct libevent_thread));

	if (!threads) {
		perror("can not allocate thread descriptors");
		exit(1);
	}

	dispatcher_thread.base = main_base;
	dispatcher_thread.thread_id = pthread_self();

	for (i = 0; i < nthreads; i++) {
		int fds[2];
		if (pipe(fds)) {
			perror("can not create notify pipe");
			exit(1);
		}

		threads[i].notify_receive_fd = fds[0];
		threads[i].notify_send_fd = fds[1];

		setup_thread(&threads[i]);
		/* Reserve three fds for the libevent base, and two for the pipe*/
		stats.reserved_fds += 5;
	}

	/*Create threads after we've done all the libevent setup*/
	for (i = 0; i < nthreads; i++) {
		create_worker(worker_libevent, &threads[i]);
	}
	
	/*wait for all the threads to set themselves up before returing*/
	pthread_mutex_lock(&init_lock);
	wait_for_thread_registration(nthreads);
	pthread_mutex_unlock(&init_lock)	
	
}

#if 0
conn *conn_new(const in sfd, enum conn_stats init_state, 
								const int event_flags,
								const int read_buffer_size, enum network_transport transport,
								struct event_base *base)
{
	conn *c = conn_from_freelist();
	
	if (NULL == c) {
		if (!(c = (conn *)calloc(1, sizeof(conn)))) {
			STATS_LOCK();
			stats.malloc_fails++;
			STATS_UNLOCK();
			fprintf(stderr, "Failed to allocate connection object\n");
			return NULL;
		}

		MEMCACHED_CONN_CREATE(c);
	}

}

#endif

void event_handler(const int fd, const short which, void *arg)
{
	conn *c;
	c = (conn *)arg;
	assert(c != NULL);
	c->which = which;

	if (fd != c->sfd) {
		if (settings.verbose > 0)
			fprintf(stderr, "catastrophic: event fd does not match conn\n");
		conn_close(c);
		return ;
	}
	drive_machine(c);

	return;
}

static void drive_machine(conn *c)
{
	bool stop = false;
	
	int sfd;
	socklen_t addrlen;
	struct sockaddr_storage addr;
	int nreqs = settings.resq_per_event;
	int res;

	const char *str;

	assert(c != NULL);

	while(!stop)
	{
		switch(c->state) {
			case conn_listening:
				addrlen = sizeof(addr);

				sfd = accept(c->sfd, (struct sockaddr *)&addr, &addrlen);

				if (sfd == -1) {
					if (error == EAGAIN || errno == EWOULDBLOCK) {
						stop = true;
					} else if (errno == EMFILE) {
						if (settings.verbose > 0) {
							fprintf(stderr, "Too many open connection\n");
						}
						accept_new_conns(false);
						stop = true;
					}

					break;
				}

				if (settings.maxconns_fast &&
						stats.curr_conns + stats.reserved_fds >= settings.maxconns - 1) {
					str = "Error, Too many open connection\r\n";
					res = write(sfd, str, strlen(str));
					close(sfd);
					STATS_LOCK();
					stats.rejected_conns++;
					STATS_UNLOCK();
				} else {
					dispatch_conn_new(sfd, conn_new_cmd, EV_READ | EV_PERSIST, DATA_BUFFER_SIZE, tcp_transport);
				}

				stop = true;
				break;
			case conn_waiting:
				if (!update_event(c, EV_READ | EV_PERSIST)) {
					if (settings.verbose > 0)
						fprintf(stderr, "Could not update event\n");
					conn_set_state(c, conn_read);
					break;
				}
				conn_set_state(c, conn_read);
				stop = true;
				break;

			case conn_read:
				res = IS_UDP(c->transport) ? try_read_udp(c) : try_read_network(c);
				switch(res)
				{
					case READ_NO_DATA_RECEIVED:
						conn_set_state(c, conn_waiting);
						break;
					case READ_DATA_RECEIVED:
						conn_set_state(c, conn_parse_cmd);
						break;
					case READ_ERROR:
						conn_set_state(c, conn_closing);
						break;
					case READ_MEMORY_ERROR:
						break;
				}
			case conn_new_cmd:
				break;
			case conn_nread:
				if (c->rlbytes == 0) {
					complete_nread(c);
					break;
				}

				if (c->rlbytes < 0) {
					if (settings.verbose) {
						fprintf(stderr, "Invalid rlbytes to read: len %d\n", c->rlbytes);
					}

					conn_set_state(c, conn_closing);
					break;
				}

				if (c->rbytes > 0) {
					int tocopy = c->rbytes > r->rlbytes ? c->rlbytes : c->rbytes;

					if (c->ritem != c->rcurr) {
						memmove(c->ritem, c->rcurr, tocopy);
					}

					c->ritem += tocopy;
					c->rlbytes -= tocopy;
					r->rcurr += tocopy;
					r->rbytes -= tocopy;
					if (c->rlbytes == 0) {
						break;
					}
				}

				res = read(c->sfd, c->ritem, c->rlbytes);
				if (res > 0) {
					pthread_mutex_lock(&c->thread->stats.mutex);
					c->thread->stats.bytes_read += res;
					pthread_mutex_unlock(&c->thread->stats.mutex);
					
					if (c->rcurr == c->ritem)
						c->rcurr += res;
						
					c->ritem += res;
					c->rlbytes -= res;
					break;
				}


				if (res == 0) {
					conn_set_state(c, conn_closing);
					break;
				}

				if (res == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
					if (!update_event(c, EV_READ | EV_PERSIST)) {
						if (settings.verbose > 0)
								fprintf(stderr, "could not update event\n");
						conn_set_state(c, conn_closing);
						break;
					}
				}

		}
	}
}
