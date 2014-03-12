#ifndef _HTTP_RESOLVE_H_INCLUDED__
#define _HTTP_RESOLVE_H_INCLUDED__

//struct dns_server record
//the dns server ip,port
//eg. infomations
#include <netinet/in.h>

struct dns_server{
	char host[200];
	int port;
	//int quick; //如果/etc/resolve.conf中的nameserver是url，quick = 0，否则, quick = 1
};

struct resolver_result{
	char *key; // 需要解析的host
	char **value; //解析后得到的ip列表
	int question_len; //把key转换成question后的长度，注意，通过strlen求直，需要加1（结尾'\0'）
};

struct resolver_st{
	struct dns_server *DServer; //dns servers
	//unsigned int DSmax; //dns servers 的最大个数
	//unsigned int DSs;//dns Server的当前个数
	struct rbtree_st *addr_rbtree; //存放查询的结果，key为查询的url，value为查询的dns结果
	struct event_base *base; //reacotr 模式
	int fd;
	int sockfd;
	struct sockaddr_in resolve;
	
};

struct resolver_st *resolver_create();
int resolver_init(struct resolver_st *resolver);
void resolve_name(struct resolver_st *resolver, unsigned char *host);
void resolver_distory(struct resolver_st *resolver);

#endif
