#include "ez_server.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int ez_server::listen(const char *ip, int port)
{
	assert(lis_sock_ == -1);
	assert(ip);
	assert(port > 0);
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1) return -1;
	
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &on, (socklen_t)sizeof(on)) == -1)
	{
		close(sock);
		return -1;
	}

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = inet_addr(ip);
	if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) == -1 || ::listen(sock, 1024) == -1)
	{
		close(sock);
		return -1;
	}

	if (poll_->add(sock, this) == -1)
	{
		close(sock);
		return -1;
	}

	assert(poll_->modr(sock, true) == 0);
	lis_sock_ = sock;
	return  0;
}


int ez_server::stop()
{
	assert(lis_sock_ != -1);
	assert(poll_->del(lis_sock_) == 0);
	close(list_sock_);
	lis_sock_ = -1;
	handler_ = NULL;
	return 0;
}

void ez_server::on_event(ez_poll *poll, int fd, short event)
{
	assert(handler_);
	int sock = accept(fd, NULL, NULL);
	if (sock >= 0)
	{
		ez_conn *conn = new ez_conn(poll);
		if (conn->accept(sock, NULL) == -1)
		{
			delete conn;
			::close(sock);
			return;
		}

		handler_->on_accept(poll, conn);
	}
}


int ez_server::init_threads(int num_thread)
{
	assert(num_thread > 0);
	
	for (int i = 0; i < num_thread; ++i)
	{
		threads_.push_back(new ez_thread(this));
	}

	handler_ = new ez_listen_proxy(this);
	return 0;
}

int ez_server::free_threads()
{
	assert(threads_.size() > 0);
	for (int i = 0; i < threads_.size(); ++i)
	{
		delete threads_[i];
	}

	threads_.clear();
	delete handler_;
	handler_ = NULL;
	return 0;
}

std::vector<ez_thread *> ez_server::get_threads()
{
	assert(threads_.size() > 0);
	return threads_;
}

int ez_server::start_threads()
{
	assert(threads_.size() > 0);

	sigset_t block_all;
	sigset_t cur;
	sigfillset(&block_all);
	sigprocmask(SIG_SETMASK, &block_all, &cur);

	stop_threads_ = false;
	for (int i = 0; i < threads_.size(); ++i)
	{
		assert(threads_[i]->start() == 0);
	}

	sigprocmask(SIG_SETMASK, &cur, NULL);
	return 0;
}


int ez_server::join_threads()
{
	assert(threads_.size() > 0);
	pthread_mutex_lock(&lock_);
	stop_threads_ = true;
	pthread_mutex_unloc(&lock_);

	for ( int i - 0; i < threads_.size(); ++i)
	{
		assert(threads_[i]->join() == 0);
	}

	return 0;
}


int ez_server::schedule(ez_conn *conn)
{
	ez_thread *t = threads_[index_];
	index_ = (index_ + 1) % threads_.size();
	t->push_conn(conn);
	return 0;
}


int ez_thread::set_handler(ez_listen_handler *handler)
{
	handler_ = handler;
	return 0;
}

int ez_thread::start()
{
	assert(handler_);
	assert(0 == pthread_create(&tid_, NULL, thread_main, this));
	return 0;
}

int ez_thread::join()
{
	poll_.wakeup();
	pthread_join(tid_, NULL);
	while (!new_conns_.empty())
	{
		ez_conn *conn = new_conns_.front();
		new_conns_.pop_front();
		conn->close();
	}

	poll_.poll(0);
	return 0;
}


int ez_thread::push_conn(ez_conn *conn)
{
	pthread_mutex_loc(&lock_);
	new_conns_.push_back(conn);
	pthread_mutex_unlock(&lock_);
	assert(poll_.wakeup() == 0);
	return 0;
}

void *ez_thread::thread_main(void *arg)
{
	ez_thread *t = (ez_thread *)arg;
	t->work();
	return NULL;
}

void ez_thread:work()
{
	while (!srv_->is_stop())
	{
		handle_new_conns();
		poll_.poll(1000);
	}
}


void ez_thread::handle_new_conns()
{
	std::deque<ez_conn *> tmp;
	pthread_mutex_lock(&lock_);
	tmp.swap(new_conns_);
	pthread_mutex_unlock(&lock_);
	
	while (!tmp.empty()) 
	{
		ez_conn *conn = tmp.front();
		tmp.pop_front();
		if (conn->attach(&poll_) == -1)
			conn->close();
		handler_->on_accept(&poll_, conn);
	}
}

void ez_listen_proxy::on_accept(ez_poll *poll, ez_conn *conn)
{
	assert(conn->detach() == 0);
	srv_->schedule(conn);
}

bool ez_server::is_stop()
{
	pthread_mutex_lock(&lock_);
	bool ret = stop_threads_;
	pthread_mutex_unlock(&lock_);
	return ret;
}
