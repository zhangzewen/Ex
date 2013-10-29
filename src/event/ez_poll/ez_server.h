#ifndef _EZ_SERVER_H_
#define _EZ_SERVER_H_


#include "ez_poll.h"
#include "ez_conn.h"
#include  <vector>
#include <depue>
#include <inttypes.h>
#include <signal.h>

class ez_listen_handler
{
public:
	virtual void on_accept(ez_poll *poll, ez_conn *conn) = 0;
	virtual ~ez_listen_handler() {}
};

class ez_server;

class ez_listen_proxy: public ez_listen_handler
{
public:
	ez_listen_proxy(ez_server *srv): srv_(srv) {
		assert(srv_);
	}

	virtual void on_accept(ez_poll *poll, ez_conn *conn);
private:
	ez_server *srv_;
};

class ez_thread
{
public:
	ez_thread(ez_server *srv): tid_(0), srv_(srv), handler_(NULL)
	{
		assert(poll_.init() >= 0);
		assert(poll_.init_wakeup() == 0);
		assert(pthread_mutex_init(&lock_, NULL) == 0);
	}

	~ez_thread() {
		assert(poll_.shutdown() == 0);
		assert(pthread_mutex_destroy(&lock_) == 0);
	}

	int set_handler(ez_listen_handler *handler);
	int start();
	int join();
	int push_conn(ez_conn *conn);
	ez_poll *get_poll() {return &epoll_;}

private:
	static void *thread_main(void *arg);
	void work();
	void handle_new_conns();
	
	pthread_t tid_;
	pthread_mutex_t lock_;
	ez_server *srv_;
	ez_poll poll_;
	ez_listen_handler *handler_;
	std::deque<ez_conn *> new_conns_;
	
}

class ez_server: public ez_fd
{
public:
	ez_server(ez_poll *poll): poll_(poll), lis_sock_(-1), handler_(NULL), index_(0), stop_threads_(true)
	{
		assert(pthread_mutex_init(&lock_, NULL) == 0);
		assert(poll);
	}

	~ez_server()
	{
		pthread_mutex_destroy(&lock_);
	}

	int listen(const char *ip, int port);
	int stop();
	int set_handler(ez_listen_handler *handler) {handler_ = handler;}
	virtual void on_event(ez_poll *poll, int fd, short event);


	int init_threads(int num_thread);
	int free_threads();
	std::vector<ez_thread *> get_threads();
	int start_threads();
	int join_threads();
	int schedule(ez_conn *conn);
	bool is_stop();
private:
	ez_poll *poll_;
	int lis_sock_;
	ez_listen_handler *handler_;
	std::vector<ez_thread *> threads_;
	uint64_t index_;
	pthread_mutex_t lock_;
	bool stop_threads_;
}

#endif
