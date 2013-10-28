#ifndef _EZ_POLL_H_
#define _EZ_POLL_H_

#include <map>
#include <inttypes.h>
#include <unistd.h>
#include <fcntl.h>


enum ez_event {
	ez_non = 0x00,
	ez_read = 0x01,
	ez_write = 0x02,
	ez_error = 0x04
};


class ez_poll;
	
class ez_fd
{
public:
	virtual void on_event(ez_poll *poll, int fd, short event) = 0;
	virtual ~ez_fd() {};
};


class ez_timer
{
public:
	virtual void on_timer(ez_poll *poll) = 0;
	virtual ~ez_timer() {};
};

class ez_poll
{
public:
	typedef std::mulitimap<uint64_t, ez_timer *>timer_map;
	typedef timer_map::interator timer_id;
	ez_poll();
	int init();
	int shutdown();
	int add(int fd, ez_fd *ezfd);
	int del(int fd);
	int modr(int fd, bool set);
	int modw(int fd, bool set);
	int poll(int timeout);
	timer_id add_timer(ez_timer *timer, int after);
	int del_timer(timer_id id);
	int init_wakeup();
	int wakeup();
private:
	int mod(int fd);
	int run_timer();
	int setnonblock(int fd);
	class ez_data
	{
	public:
		int fd_;
		ez_fd *ezfd_;
		short event_;
		bool closed_;
	};
	
	class ez_wakefd: public ez_fd
	{
	public:
		virtual void on_event(ez_poll *poll, int fd, short event);
	};
	
	bool inloop_;
	int epfd_;
	int maxfd_;
	ez_data **fd2data_;
	int wake_fd_[2];
	ez_wakefd *wake_;
	int closed_size_;
	int closed_count_;
	ez_data **closed_;
	
	timer_map timers_;
};



#endif
