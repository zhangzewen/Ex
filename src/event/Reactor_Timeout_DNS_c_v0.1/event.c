
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/time.h>
#include <assert.h>

#include "http_epoll.h"
#include "event.h"
#include "list.h"
#include "RBTree.h"
#include "timer.h"

struct eventop epollops = {
    .name = "epoll",
    .init = epoll_init,
    .add = epoll_add,
    .del = epoll_del,
    .dispatch = epoll_dispatch,
    .dealloc = epoll_dealloc,
};

struct event_base *current_base = NULL;


static void event_queue_insert(struct event_base *, struct event *, int);

static void event_queue_remove(struct event_base *, struct event *, int);

static int event_haveevents(struct event_base *);

static void event_add_timer(struct event_base *, struct event *);

static void event_del_timer(struct event_base *, struct event *);

static int gettime(struct event_base *base, struct timeval *tp);

static int timeout_next(struct event_base *base, struct timeval **tv_p)
{

	struct timeval now;
	struct event *ev;
	struct timeval *tv = *tv_p;
	rbtree_node_t *tmp;
	
	if ((tmp = base->timeout.min(base->timeout.root)) == NULL) {
		*tv_p = NULL;
		return 0;
	}

	if (gettime(base, &now) == -1)
		return -1;
/* 这还有一步，通过tmp找到ev*/
	ev =(struct event*)tmp->data;
	
#if 0
	fprintf(stderr, "[%s:%d]:ev->ev_timeout.tv_sec = %lld, ev->ev_timeout.tv_usec = %lld",
					__func__,
					__LINE__,
					ev->ev_timeout.tv_sec,
					ev->ev_timeout.tv_usec);
	fprintf(stderr, "[%s:%d]:now.tv_sec = %lld, now.tv_usec = %lld",
					__func__,
					__LINE__,
					now.tv_sec,
					now.tv_usec);
#endif
	if (!timer_cmp(ev->ev_timeout, now)) {
		timer_reset(tv);
		return 0;
	}

	timer_sub(&ev->ev_timeout, &now , tv);
#if 0
	fprintf(stderr, "[%s:%d]:tv.tv_sec = %lld, tv.tv_usec = %lld",
					__func__,
					__LINE__,
					tv->tv_sec,
					tv->tv_usec);
#endif
	return 0;
}
struct event_base *event_init(void)
{
	struct event_base *base = event_base_new();
	
	if(base != NULL) {
		current_base = base;
	}

	return (base);
}

static int gettime(struct event_base *base, struct timeval *tp)
{
	//如果tv_cache事件缓存已设置，就直接使用
	
	if (base->tv_cache.tv_sec) {
		*tp = base->tv_cache;
		return 0;
	}
	return (gettimeofday(tp, NULL));
}


struct event_base *event_base_new(void)
{
	struct event_base *base;
	
	if ((base = calloc(1, sizeof(struct event_base))) == NULL) {
		fprintf(stderr, "%s: calloc\n", __func__);
	}
	
	gettime(base, &base->event_tv);
	
	rbtree_init(&base->timeout);
	
	INIT_LIST_HEAD(&base->eventqueue);
	INIT_LIST_HEAD(&base->activequeue);

	base->evbase = NULL;
	base->evsel = &epollops;
	base->evbase = base->evsel->init();

	if(base->evbase == NULL) {
		fprintf(stderr, "%s: no event mechanism available\n", __func__);
	}
	return (base);
}



static void event_process_active(struct event_base *base)
{
	struct event *ev;
	struct event *tmp;
	short ncalls;
	

	if(list_empty(&base->activequeue)) {
		return;
	}

	list_for_each_entry_safe(ev, tmp,&base->activequeue, active_list) {
		if (ev->ev_events & EV_PERSIST) {
			event_queue_remove(base, ev, EVLIST_ACTIVE);
		}else{
			event_del(ev);
		}

		ncalls = ev->ev_ncalls;
		ev->ev_pncalls = &ncalls;
		while(ncalls) {
			ncalls--;
			ev->ev_ncalls = ncalls;
			(*ev->ev_callback)((int)ev->ev_fd, ev->ev_res, ev->ev_arg);
		}

		if(list_empty(&base->activequeue)) {
			return;
		}
	}
}


int event_dispatch(void)
{
	return (event_loop(0));
}

int event_base_dispatch(struct event_base *event_base)
{
	return (event_base_loop(event_base, 0));
}

const char *event_base_get_method(struct event_base *base)
{
	return (base->evsel->name);
}



int event_loop(int flags)
{
	return event_base_loop(current_base, flags);
}

void timeout_process(struct event_base *base)
{
	struct timeval now;
	struct event *ev;
	struct rbtree_node_st *tmp;
	

	if (base->timeout.empty(base->timeout.root)) {
		return ;
	}
#if 0 
	fprintf(stderr, "[%s:%d]:base->tv_cache->tv_sec = %lld,base->tv_cache->tv_usec = %lld, base->event_tv.tv_sec = %lld, base->event_tv.usec = %lld\n",
					 __func__,
					 __LINE__,
					base->tv_cache.tv_sec,
					base->tv_cache.tv_usec,
					base->event_tv.tv_sec,
					base->event_tv.tv_usec);
#endif
	gettime(base, &now);
#if 0
	fprintf(stderr, "[%s:%d]:base->tv_cache->tv_sec = %lld,base->tv_cache->tv_usec = %lld, base->event_tv.tv_sec = %lld, base->event_tv.usec = %lld\n",
					 __func__,
					 __LINE__,
					base->tv_cache.tv_sec,
					base->tv_cache.tv_usec,
					base->event_tv.tv_sec,
					base->event_tv.tv_usec);
	fprintf(stderr, "[%s:%d]:now.tv_sec = %lld, now.tv_usec = %lld\n",
					__func__,
					__LINE__,
					now.tv_sec,
					now.tv_usec);
#endif
	while ((tmp = base->timeout.min(base->timeout.root))) {
		ev = (struct event *)tmp->data;
		if (!timer_cmp(now, ev->ev_timeout)) { //还没有超时
			break;
		}
		//本事件已经超时
	 /*delete this event from the I/O queues*/ 
		event_del(ev);

		event_active(ev, EV_TIMEOUT, 1);
	}
}
/*
	如果系统支持monotonic时间，该时间是系统从boot后到现在所经过的时间，因此不需要执行校正。
根据前面的代码逻辑，如果系统不支持monotonic时间，用户可能会手动的调整时间，如果时间被向前调整了
（MS前面第7部分讲成了向后调整，要改正），比如从5点调整到了3点，那么在时间点2取得的值可能会小于
上次的时间，这就需要调整了，下面来看看校正的具体代码，由函数timeout_correct()完成


 在调整小根堆时，因为所有定时事件的时间值都会被减去相同的值，因此虽然堆中元素的时间键值改变了，但是
相对关系并没有改变，不会改变堆的整体结构。因此只需要遍历堆中的所有元素，将每个元素的时间键值减去相同
的值即可完成调整，不需要重新调整堆的结构。
当然调整完后，要将event_tv值重新设置为tv_cache值了
*/

static void timeout_correct(struct event_base *base, struct timeval *tv)
{
#if 0
	fprintf(stderr, "[%s:%d]:base->tv_cache->tv_sec = %lld,base->tv_cache->tv_usec = %lld, base->event_tv.tv_sec = %lld, base->event_tv.usec = %lld\n",
					 __func__,
					 __LINE__,
					base->tv_cache.tv_sec,
					base->tv_cache.tv_usec,
					base->event_tv.tv_sec,
					base->event_tv.tv_usec);

	fprintf(stderr, "[%s:%d]:tv->tv_sec = %lld, tv->tv_usec = %lld",
					__func__,
					__LINE__,
					tv->tv_sec,
					tv->tv_usec);
#endif
	gettime(base, tv);// tv <---- tv_cache
	//根据前面的分析可以知道event_ev应该小于tv_cache
	//如果tv < event_tv表明用户向前调整时间了，需要校正时间	
#if 0
	fprintf(stderr, "[%s:%d]:base->tv_cache->tv_sec = %lld,base->tv_cache->tv_usec = %lld, base->event_tv.tv_sec = %lld, base->event_tv.usec = %lld\n",
					 __func__,
					 __LINE__,
					base->tv_cache.tv_sec,
					base->tv_cache.tv_usec,
					base->event_tv.tv_sec,
					base->event_tv.tv_usec);
	fprintf(stderr, "[%s:%d]:tv->tv_sec = %lld, tv->tv_usec = %lld",
					__func__,
					__LINE__,
					tv->tv_sec,
					tv->tv_usec);
#endif
	                                                       	
	if (timer_cmp(*tv, base->event_tv)) {                  	
		base->event_tv = *tv;                                	
		return ;                                             	
	}
#if 0
	//之所以要注释掉这段代码，是因为我们暂且认为在程序运行期间，时间没有被人为的调整
	//计算时间差
	
	evutil_timeersub(&base->event_tv, tv, &off);
	
	//调整定时事件最小堆
	
	pev = base->timeout.p;
	size = base->timeout.n;

	for (; size-- > 0; ++pev) {
		struct timeval *ev_tv = &(**pev).ev_timeout;
		evutil_timersub(ev_tv, &off, ev_tv);
	}

	base->event_tv = *tv; //更新event_tv为tv_cache
#endif
}
/*
	时间event_tv指示了dispatch()上次换回，也就是I/O事件就绪时的时间，第一次进入循环时，由于tv_cache被清空，因此gettime()执行系统调用获取当前系统时间，而后将会更细为tv_cache指示的时间
	时间tv_cache在dispatch()返回后被设置为当前系统时间，因此它缓存了本次I/O事件就绪时的时间（event_tv）
	从代码逻辑里可以看出event_tv获取的是tv_cache上一次的值，因此event_tv应该小于tv_cache的值
	设置时间缓存的优点是不必每次获取时间都执行系统调用，这是一个相对费时的操作。在上面标注的时间点2到时间点1的这段时间（处理就绪事件时），调用gettime取得的都是tv_cache缓存的时间
*/

int event_base_loop(struct event_base *base, int flags)
{
	struct eventop *evsel = base->evsel;
	struct epoll_loop *evbase = base->evbase;
	int res;
	struct timeval tv;
	struct timeval *tv_p;

	assert(flags == 0);	
	/*clear time cache*/
	//清空时间缓存
	base->tv_cache.tv_sec = 0;
#if 0
	fprintf(stderr, "[%s:%d]:base->tv_cache.tv_sec = %lld,base->tv_cache.tv_usec = %lld, base->event_tv.tv_sec = %lld, base->event_tv.tv_usec = %lld\n",
					 __func__,
					 __LINE__,
					base->tv_cache.tv_sec,
					base->tv_cache.tv_usec,
					base->event_tv.tv_sec,
					base->event_tv.tv_usec);
#endif
	
	while(1) {
		timeout_correct(base, &tv);//时间矫正
#if 0
	fprintf(stderr, "[%s:%d]:base->tv_cache.tv_sec = %lld,base->tv_cache.tv_usec = %lld, base->event_tv.tv_sec = %lld, base->event_tv.tv_usec = %lld\n",
					 __func__,
					 __LINE__,
					base->tv_cache.tv_sec,
					base->tv_cache.tv_usec,
					base->event_tv.tv_sec,
					base->event_tv.tv_usec);
	fprintf(stderr, "[%s:%d]:tv.tv_sec = %lld, tv.tv_usec = %lld\n",
					__func__,
					__LINE__,
					tv.tv_sec,
					tv.tv_usec);
#endif
		tv_p = &tv;

		if (!base->event_count_active ) {
			timeout_next(base, &tv_p);
#if 0
	fprintf(stderr, "[%s:%d]:base->tv_cache.tv_sec = %lld,base->tv_cache.tv_usec = %lld, base->event_tv.tv_sec = %lld, base->event_tv.tv_usec = %lld\n",
					 __func__,
					 __LINE__,
					base->tv_cache.tv_sec,
					base->tv_cache.tv_usec,
					base->event_tv.tv_sec,
					base->event_tv.tv_usec);
	fprintf(stderr, "[%s:%d]:tv_p->tv_sec = %lld, tv_p->tv_usec = %lld\n",
					__func__,
					__LINE__,
					tv_p->tv_sec,
					tv_p->tv_usec);
#endif
		} else {
			/*
				if we have active events, we just poll new events
				without waiting
			*/
			timer_reset(&tv);
		}
				
		if (!event_haveevents(base)) {
			fprintf(stderr, "%s: no events registered.\n", __func__);
			return 1;
		}

		/*update last old time*/
		//更新event_tv到ev_cache指示的时间或者当前时间
		//event_ev <<---- tv_cache
		gettime(base, &base->event_tv);
#if 0
	fprintf(stderr, "[%s:%d]:base->tv_cache.tv_sec = %lld,base->tv_cache.tv_usec = %lld, base->event_tv.tv_sec = %lld, base->event_tv.tv_usec = %lld\n",
					 __func__,
					 __LINE__,
					base->tv_cache.tv_sec,
					base->tv_cache.tv_usec,
					base->event_tv.tv_sec,
					base->event_tv.tv_usec);
#endif
		/*clear time cache*/
		//清除时间缓存 -- 时间点1
		base->tv_cache.tv_sec = 0;
		
		//等待I/O事件就绪
		res = evsel->dispatch(evbase, tv_p);

		if (res == -1) {
			return -1;
		}

		//缓存tv_cache存储了当前时间的值 --时间点2
		//tv_cache < ----now
		gettime(base, &base->tv_cache);
#if 0
	fprintf(stderr, "[%s:%d]:base->tv_cache.tv_sec = %lld,base->tv_cache.tv_usec = %lld, base->event_tv.tv_sec = %lld, base->event_tv.tv_usec = %lld\n",
					 __func__,
					 __LINE__,
					base->tv_cache.tv_sec,
					base->tv_cache.tv_usec,
					base->event_tv.tv_sec,
					base->event_tv.tv_usec);
#endif

		timeout_process(base);

		if (base->event_count_active) {
			event_process_active(base);
		}
	}
	//退出时也要清空时间缓存
	base->tv_cache.tv_sec = 0;

	fprintf(stderr, "%s: asked to terminate loop.\n", __func__);

	return 0;
}


int event_add(struct event *ev, const struct timeval *tv)
{
	struct event_base *base = ev->ev_base;
	struct eventop *evsel = base->evsel;
	struct epoll_loop *evbase =  base->evbase;

	int res = 0;
#if 0	
	fprintf(stderr, "event_add: event : %p, %s%s%scall %p\n",
					 ev,
					 ev->ev_events&EV_READ ? "EV_READ" : " ",
					 ev->ev_events&EV_WRITE ? "EV_WRITE" : " ",
					 tv ? "EV_TIMEOUT" : " ",
					 ev->ev_callback
					);
#endif

/*
	这个没有弄懂，或许看看min_heap的实现
	prepare for timeout insertion further below, if we get a 
	failure on any step, we should not change any state.
*/	
#if 0
	if (tv != NULL && !(ev->ev_flags & EVLIST_TIMEOUT)) {
		if (min_heap_reserve(&base->timeout,
			1 + min_heap_size(&base->timeout)) == -1) {
			return -1;
		}
	}
#endif
	
	if ((ev->ev_events & (EV_READ | EV_WRITE)) &&
		!(ev->ev_flags & (EVLIST_INSERTED | EVLIST_ACTIVE))) {
		res = evsel->add(evbase, ev);

		if (res != -1) {
			event_queue_insert(base, ev, EVLIST_INSERTED);
		}
	}

/*
	we should change the timeout state only if the previous event
	addition succeeded.
*/

	if (res != -1 && tv != NULL) {
		struct timeval now;

		/*
			we already reserved memory above for the case where we 
			are not replacing an exisiting timeout
		*/

		if (ev->ev_flags & EVLIST_TIMEOUT) {
			event_queue_remove(base, ev, EVLIST_TIMEOUT);
		}


		/*
			check if it is active due to a timeout. Rescheduling
			this timeout before the callback can be executed
			removes it from the active list.
		*/

		if ((ev->ev_flags & EVLIST_ACTIVE) && (ev->ev_res & EV_TIMEOUT)) {
			/*
				See uf we are just active executing this
				event in a loop
			*/

			if (ev->ev_ncalls && ev->ev_pncalls) {
				/*abort loop*/
				*ev->ev_pncalls = 0;
			}
			event_queue_remove(base, ev, EVLIST_ACTIVE);
		}

		gettime(base, &now);
		
#if 0
		fprintf(stderr, "[%s]:%d:base->tv_cache.tv_sec = %lld,base->tv_cache.tv_usec = %lld, now.tv_sec = %lld, now.tv_usec = %lld\n",
						 __func__,
						 __LINE__,
						base->tv_cache.tv_sec,
						base->tv_cache.tv_usec,
						now.tv_sec,
						now.tv_usec);
#endif
		
		timer_add(&now, tv, &ev->ev_timeout);
#if 0
		fprintf(stderr, "[%s]:%d:tv->tv_sec = %lld,tv->tv_usec = %lld, now.tv_sec = %lld, now.tv_usec = %lld,ev->ev_timeout.tv_sec = %lld, ev->ev_timeout.tv_usec = %lld\n",
						 __func__,
						 __LINE__,
						tv->tv_sec,
						tv->tv_usec,
						now.tv_sec,
						now.tv_usec,
						ev->ev_timeout.tv_sec,
						ev->ev_timeout.tv_usec);
#endif
	//	fprintf(stderr, "event_add: timeout in %ld seconds, call %p\n", tv->tv_sec, ev->ev_callback);
		event_queue_insert(base, ev, EVLIST_TIMEOUT);
	}
	
	return (res);
}


void event_set(struct event *ev, int fd, short events, void (*callback)(int, short, void *), void *arg, char *name)
{
	ev->ev_base = current_base;
	ev->ev_callback = callback;
	ev->ev_arg = arg;
	ev->ev_fd = fd;
	ev->ev_events = events;
	ev->ev_res = 0;
	ev->ev_flags = EVLIST_INIT;
	ev->ev_ncalls = 0;
	ev->ev_pncalls = NULL;
	ev->name = name;
	
	
	INIT_LIST_HEAD(&ev->event_list);
	INIT_LIST_HEAD(&ev->active_list);
}


int event_del(struct event *ev)
{
	struct event_base *base;
	
	struct eventop *evsel;
	
	struct epoll_loop *evbase;
	
	if (ev->ev_base == NULL) {
		return -1;
	}

	base = ev->ev_base;
	evsel = base->evsel;
	evbase = base->evbase;
	
	if (ev->ev_ncalls && ev->ev_pncalls) {
		*ev->ev_pncalls = 0;
	}


	if (ev->ev_flags & EVLIST_ACTIVE) {
		event_queue_remove(base, ev, EVLIST_ACTIVE);
	}

	if (ev->ev_flags & EVLIST_INSERTED) {
		event_queue_remove(base, ev, EVLIST_INSERTED);
		return (evsel->del(evbase, ev));
	}

	if (ev->ev_flags & EVLIST_TIMEOUT) {
		event_queue_remove(base, ev, EVLIST_TIMEOUT);
	}

	return 0;	
}

void event_active(struct event *ev, int res, short ncalls)
{
	//当事件处在激活队列中，可能会有不同是事件，把这些事件都加在一起
	if (ev->ev_flags & EVLIST_ACTIVE) {
		ev->ev_res |= res;
		return ;
	}

	ev->ev_res = res;
	ev->ev_ncalls = ncalls;
	ev->ev_pncalls = NULL;
	event_queue_insert(ev->ev_base, ev, EVLIST_ACTIVE);
}


void event_queue_insert(struct event_base *base, struct event *ev, int queue)
{
	if (ev->ev_flags & queue) { //进行与运算，看着个事件是不是在队列中（queue的值为EVLIST_INSERTED等待队列,数值为EVLIST_ACTIVE为事件就绪队列）
		//这个在libevent1.X中的说明是可以的* Double insertion is possible for active events *
		if (queue & EVLIST_ACTIVE) {
			return ;
		}
	}
	

	if (~ev->ev_flags & EVLIST_INTERNAL) {
		base->event_count++;
	}
	

	ev->ev_flags |= queue; //打上标记，标示着个事件已经在这个queue队列中的

	switch(queue) {
		case EVLIST_INSERTED:
			list_add_tail(&ev->event_list, &base->eventqueue);
			break;
		case EVLIST_ACTIVE:
			base->event_count_active++;
			list_add_tail(&ev->active_list, &base->activequeue);
			break;
		case EVLIST_TIMEOUT:
			event_add_timer(base, ev);
			break;
		default:
			fprintf(stderr, "%s: unknown queue %x\n", __func__, queue);
	}
}



void event_queue_remove(struct event_base *base, struct event *ev, int queue)
{
	if (!(ev->ev_flags & queue)) {
		fprintf(stderr, "%s: %p(fd %d) not no queue %x\n", __func__, ev, ev->ev_fd, queue);
	}

	if (~ev->ev_flags & EVLIST_INTERNAL) {
		base->event_count--;
	}

	ev->ev_flags &= ~queue;

	switch (queue) {
		case EVLIST_INSERTED:
			list_del(&ev->event_list);
			break;
		case EVLIST_ACTIVE:
			base->event_count_active--;
			list_del(&ev->active_list);
			break;
		case EVLIST_TIMEOUT:
			//min_heap_erase(&base->timeout, ev);
			event_del_timer(base, ev);
			break;

		default:
			fprintf(stderr, "%s: unknown queue %x\n", __func__, queue);
	}
}



int event_haveevents(struct event_base *base)
{
	return base->event_count > 0;
}

static void event_add_timer(struct event_base *base, struct event *ev)
{
	uintptr_t key = 0;
	
	if (!timer_isset(&ev->ev_timeout)) {
		return ;
	}

	key = ev->ev_timeout.tv_sec * 1000 + ev->ev_timeout.tv_usec / 1000; //取毫秒级别做key标示
	
	base->timeout.root = base->timeout.insert(key, (void*)ev, base->timeout.root);
	
	
		
	
}
static void event_del_timer(struct event_base* base, struct event *ev)
{
	uintptr_t key = 0;	

	if (!timer_isset(&ev->ev_timeout))	 {
		return ;
	}

	key = ev->ev_timeout.tv_sec * 1000 + ev->ev_timeout.tv_usec / 1000;

	base->timeout.root = base->timeout.erase(key, base->timeout.root);
	return;
}
