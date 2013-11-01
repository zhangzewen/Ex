#ifndef __REACTOR_TIMEOUT_C_EVENT_BASE_H_INCLUDED__
#define __REACTOR_TIMEOUT_C_EVENT_BASE_H_INCLUDED__

#include <sys/time.h>
#include "evbuf.h"

#include "list.h"
#include "evutil.h"
#include "RBTree.h"
/*以下这几个宏定义是给ev_flags标记的，表明事件当前的状态*/
#define EVLIST_TIMEOUT 0X01 /*event在time堆中*/
#define EVLIST_INSERTED 0X02/*event已经在注册事件链表中*/
#define EVLIST_SIGNAL 0X04/*未见使用*/
#define EVLIST_ACTIVE 0X08/*event在激活链表中*/
#define EVLIST_INTERNAL 0X10/*内部使用标记*/
#define EVLIST_INIT 0X80/*event已经被初始化*/

#define EVLIST_ALL (0Xf000 | 0x9f)

#define EV_TIMEOUT 0X01 //=====1
#define EV_READ 0X02   //======10
#define EV_WRITE 0X04  //=====>100
#define EV_SIGNAL 0X08 //=====>1000
#define EV_PERSIST 0X10 //====>10000

#define EVLOOP_NONBLOCK 0x02  /**< Do not block. */
#include <sys/epoll.h>


#define MAX_EPOLL_TIMEOUT_MSEC (35*60*1000)
#define INITIAL_NFILES 32
#define INITIAL_NEVENTS 32
#define MAX_NEVENTS 4096

struct event_base;

struct event_epoll{
	struct event *read;
	struct event *write;
};

struct epoll_loop{
	struct event_epoll *fds;
	int nfds;
	struct epoll_event *events;
	int nevents;
	int epfd;
};

struct event{
  struct event_base *ev_base;
	struct timeval ev_timeout;

  int ev_fd;
	struct evbuffer *buffer;	
  short ev_events;
  short ev_ncalls;
  short *ev_pncalls;

	struct rb_node_t timer; //红黑树节点
	unsigned int istimeout; //是否timeout了，是：1，否：0

  void (*ev_callback)(int, short, void *arg);
  void *ev_arg;

  int ev_res;
  int ev_flags;
  struct list_head event_list;
	struct list_head active_list;
};

struct eventop {
	const char *name;
	struct epoll_loop *(*init)(struct event_base *);
	int (*add)(struct epoll_loop *loop, struct event *);
	int (*del)(struct epoll_loop *loop, struct event *);
	int (*dispatch)(struct event_base *, struct epoll_loop *loop, struct timeval *tv);
	void (*dealloc)(struct event_base *, struct epoll_loop *loop);
};

struct event_base {
	struct eventop *evsel;
	struct epoll_loop *evbase;
	int event_count;
	int event_count_active;
	
	int event_gotterm;
	int event_break;
	
	struct rb_tree timeout; //维护一个红黑树来管理时间
	struct timeval tv_cache;
	struct timeval event_tv;

	int nactivequeues;
	
	struct list_head activequeue;
	struct list_head eventqueue;
};

#endif
