#ifndef __HTTP_LOG_H_INCLUDED_
#define __HTTP_LOG_H_INCLUDED_

#if 0
enum LEVEL{
	TRACE = 0,
	DEBUE,
	INFO,
	WARN,
	ERROR,
	FATAL
};
struct log_node{
	char buff[BUFSIZ];
	LEVEL log_level;
	struct list_head;
};
//队列A挂满了就立马写到文件中，同时把log挂到队列B上，如此循环
//单独开辟一个线程用于log写入文件处理,应用程序只需把log挂上去就行了，生产者与消费者模型
struct log{
	int count_a;
	int count_b;
	int max_a;
	int max_b;
	struct list_head log_head_A;
	struct list_head log_head_B;
};


struct log* creat_log(unsigned int a, unsigned b)
{
	
}
void log_add(struct list *head, struct list *node );
void log_free(struct list *node);
void log_process(void *arg)
{

	while(1) {
	}
	
}
#endif

#define _EVENT_LOG_DEBUG 0
#define _EVENT_LOG_MSG 1
#define _EVENT_LOG_WARN 2
#define _EVENT_LOG_ERR 3

void event_err(int eval, const char *fmt, ...);
void event_warn(const char *fmt, ...);
void event_errx(int eval, const char *fmt, ...);
void event_warnx(const char *fmt, ...);
void event_msgx(const char *fmt, ...);
void _event_debugx(const char *fmt, ...);

#endif
