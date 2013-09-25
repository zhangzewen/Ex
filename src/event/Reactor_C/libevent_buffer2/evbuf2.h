#ifndef _EVBUF2_H_INCLUDED__
#define _EVBUF2_H_INCLUDED__


#define MIN_BUFFER_SIZE 512

struct evbuffer_cb_entry{
	TAILQ_ENTRY(evbuffer_cb_entry) next;
	
	union{
		evbuffer_cb_func cb_func;
		evbuffer_cb cb_obsolete;
	} cb;

	void *cbarg;
	ev_uint32_t flags;
};

struct bufferevent;
struct evbuffer_chain;
struct evbuffer {
	struct evbuffer_chain *first;
	struct evbuffer_china *last;
	
	struct evbuffer_chain **last_with_datap;
	size_t total_len;
	size_t n_add_for_cb;
	size_t n_del_for_cb;
	void *lock;
	unsigned own_lock : 1;
	unsigned freeze_start : 1:
	unsigned freeze_end : 1;
	unsigned deferred_cbs : 1;
	ev_uint32_t flags;
	struct deferred_cb_queue *cb_queue;
	
	int refcnt;
	struct deferred_cb deferred;
	TAILQ_HEAD(evbuffer_cb_queue, evbuffer_cb_entry) callbacks;

	struct bufferevent *parent;
};

struct evbuffer_chain {
	struct evbuffer_chain *next;
	size_t buffer_len;
	ev_off_t misalign;
	size_t off;
	unsigned flags;
#define EVBUFFER_MMAP 0x0001
#define EVBUFFER_SENDFILE 0X0002
#define EVBUFFER_REFERENCE	0x0004
#define EVBUFFER_IMMUTABLE 0x0008
#define EVBUFFER_MEM_PINNED_R 0x0010
#define EVBUFFER_MEM_PINNED_W	0x0020
#define EVBUFFER_MEM_PINNED_ANY (EVBUFFER_MEM_PINNED_R | EVBUFFER_MEM_PINNED_W)

#define EVBUFFER_DANGLING 0x0040

	unsigned char *buffer;
};

struct evbuffer_chain_fd {
	int fd;
};

struct evbuffer_chain_reference{
	evbuffer_ref_cleanup_cb cleanupfn;
	void *extra;
};

#define EVBUFFER_CHAIN_SIZE sizeof(struct evbuffer_chain)

#define EVBUFFER_CHAIN_EXTRA(t, c) (t *)((struct evbuffer_chain *)(c) + 1)
#define ASSERT_EVBUFFER_LOCKED(buffer) EVLOCK_ASSERT_LOCKED((buffer)->lock)
#endif
