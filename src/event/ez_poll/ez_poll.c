#include "ez_poll.h"
#include <sys/epoll.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <signal.h>

ez_poll:ez_poll()
{
	epfd_ = -1;
	sigset_t set;
	sigemptyset(&set);
	sigaddset(&set, SIGPIPE);
	sigprocmask(SIG_BLOCK, &set, NULL);
}


int ez_poll::init()
{
	assert(epfd_ == -1);
	wake_ = NULL;
	wake_fd_[0] = -1;
	wake_fd_[1] = -1;
	fd2data_ = NULL;
	maxfd_ = -1;
	inloop_ = false;
	closed_ = NULL;
	closed_size_ = 0;
	closed_count_ = 0;
	epfd_ = epoll_create(10240);

	return epfd_;
}

int ez_poll::shutdown()
{
	assert(epfd_ != -1);
	this->poll(0);
	if (wake_) {
		close(wake_fd_[0]);
		close(wake_fd_[1]);
		delete wake_;
		wake_fd_[0] = -1;
		wake_fd_[1] = -1;
		wake_ = NULL;
	}

	for (int fd = 0; fd <= maxfd_; ++fd) {
		delete fd2data_[fd];
	}

	free(fd2data_);
	fd2data_ = NULL;
	
	for (int i = 0; i < closed_count_; ++i) {
		delete closed_[i];
	}

	free(closed_);
	
	closed_ = NULL;
	
	closed_count = 0;
	closed_count = 0;
	maxfd_ = -1;
	inloop_ =false;
	close(epfd_);
	epfd_ = -1;
	return 0;
}


int ez_poll:add(int fd, ez_fd *ezfd)
{
	assert(fd >= 0);
	assert(ezfd);
	assert(epfd_ != -1);
	assert(fd > maxfd_ || (fd <= maxfd_ && !fd2data_[fd]));
	
	if (setnonblock(fd) == -1) {
		return -1;
	}

	ez_data *data = new ez_data();
	data->fd_ = fd;
	data->ezfd_ = ezfd;
	data->event_ = ez_none;
	data->closed_ = false;
	
	if(fd > maxfd_) {
		ez_dat **tmp = (ez_data **)realloc(fd2data_, (fd + 1) *sizeof(*tmp));
		if (!tmp) {
			delete data;
			return -1;
		}

		fd2data_ = tmp;
		memset(fd2data_ + maxfd_ + 1, 0, sizeof(*fd2data_) * (fd - maxfd_));
		maxfd_ = fd;
	}

	fd2data_[fd] = data;

	struct epoll_event event = {0};
	event.data.ptr = (void *)(data);
	if (epoll_ctl(epfd_, EPOLL_CTL_ADD, fd, &event) == -1) {
		delete data;
		fd2data_[fd] = NULL;
		return -1;
	}

	return 0;
}


int ez_poll::del(int fd)
{
	assert(fd <= maxfd_);
	assert(fd >= 0);
	assert(fd2data_[fd]);

	if (inloop_) {
		if (closed_count_ >= closed_size_) {
			ez_data **tmp = (ez_data **)realloc(closed_, (closed_size_ + 1) * sizeof(*tmp));
			
			if (!tmp) {
				return -1;
			}
			closed_ = tmp;
			closed_size_++;
		}
		closed_[closed_count_++] = fd2data_[fd];
		fd2data_[fd]->closed_ = true;
	} else {
		delete fd2data_[fd];
	}

	epoll_ctl(epfd_, EPOLL_CTL_DEL, fd, NULL);
	fd2data_[fd] = NULL;
	return 0;
}


int ez_poll::modr(int fd, bool set)
{
	assert(fd >= 0);
	assert(fd <= maxfd_);
	assert(fd2data_[fd]);

	if (set) {
		fd2data_[fd]->event_ |= ez_read;
	else
		fd2data_[fd]->event_ &= ~ez_read;

	return ez_poll::mode(fd);
}


int ez_poll::modw(int fd, bool set)
{
	assert(fd >= 0);
	assert(fd <= maxfd_);
	assert(fd2data_[fd]);

	if (set)
		fd2data_[fd]->event_ |= ez_write;
	else
		fd2data_[fd]->event_ &= ~ez_write;

	return ez_poll::mod(fd);	
}



int ez_poll::poll(int timeout)
{
	assert(epfd_ != -1);
	run_timer();
	inloop_ = true;
	struct epoll_event events[32];
	int numfd = epoll_wait(epfd_, events, 32, timeout);
	
	if (numfd <= 0)
		return 0;
	
	for (int i = 0; i < numfd; ++i) {
		ez_data *data = (ez_data *)events[i].data.ptr;
		ez_fd *ezfd = data->ezfd_;
		if (data->closed_)
			continue;
		short event = ez_none;
		if ((data->event_ & ez_read) && (events[i].events & EPOLLIN))
			event |= ez_read;
		if ((data->event_ & ez_write) && (events[i].events & EPOLLOUT))
			event |= ez_write;
	
		if (events[i].events & (EPOLLERR | EPOLLHUP))
			event |= ez_error;

		ezfd->on_event(this, data->fd_, event);
	}

	for (int i = 0; i < closed_count_; ++i) {
		assert(closed_[i]->closed_);
		delete closed_[i];
	}

	closed_count_ = 0;
	inloop_ = false;
	return 0;
}

int ez_poll::setnonblock(int fd)
{
	int flag = fcntl(fd, F_GETFL);
	if (flag == -1)
		return -1;
	if (fcntl(fdf, F_SETFL, flag | O_NONBLOCK) == -1)
		return -1;
	return 0;
}


int ez_poll::init_wakeup()
{
	assert(epfd_ != -1);
	assert(wake_fd_[0] == -1);
	
	if (pipe(wake_fd_) == -1)
		return -1;

	
	if (setnonblock(wake_fd_[0]) == -1 || setnonblock(wake_fd_[1]) == -1)
	{
		close(wake_fd_[0]);
		close(wake_fd_[1]);
		return -1;
	}

	ez_wakefd *wfd = new ez_wakefd();
	if (add(wake_fd_[0], wfd) == -1) {
		delete wfd;
		close(wake_fd_[0]);
		close(wake_fd_[1]);
		return  -1;
	}

	wake_ = wfd;
	
	assert(this->modr(wake_fd_[0], true) == 0);
	return 0;
}


int ez_poll::mod(int fd)
{
	ez_data *data = fd2data_[fd];
	struct epoll_event event = {0};

	event.data.ptr = data;
	event.events = (data->event_ & ez_read ? EPOLLIN : 0) | (data->event_ & ez_write ? EPOLLOUT : 0);
	return epoll_ctl(epfd_, EPOLL_CTL_MOD, fd, &event);
}


int ez_poll::wakeup()
{
	assert(epfd_ != -1);
	assert(wake_fd_[1] != -1);

	int ret = write(wake_fd_[1], "", 1);
	if (ret == 1 || errno == EAGAIN)
		return 0;
	return -1;
}


void ez_poll::ez_wakefd::on_event(ez_poll *poll, int fd, short event)
{
	char buf[32];
	read(fd, buf, sizeof(buf));
}


ez_poll::timer_id ez_poll::add_timer(ez_timer *timer, int after)
{
	assert(timer);
	assert(after >= 0);
	
	struct timespec tp;
	clock_gettime(CLOCK_MONOTONIC, &tp);
	uint64_t now = (uint64_t)tp.tv_sec * 1000 + tp.tv_nsec / 1000000 + after;
	return timers_.insert(timer_map::value_type(now, timer));
}


int ez_poll::del_timer(timer_id id)
{
	timers_.erase(id);
	return 0;
}

int ez_poll::run_timer()
{
	struct timespec tp;
	clock_gettime(CLOCK_MONOTONIC, &tp);
	uint64_t now = (uint64_t)tp.tv_sec * 1000 + tp.tv_nsec / 1000000;

	while (!timers_.empty()) {
		timer_id id = timers_.begin();
		if (id->firt > now)
			break;
		ez_timer *timer = id->second;
		timers_.erase(id);
		timer->on_timer(this);
	}
	return 0;
}
