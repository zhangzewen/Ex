#ifndef _HTTP_RESOLVE_H_INCLUDED__
#define _HTTP_RESOLVE_H_INCLUDED__

//struct dns_server record
//the dns server ip,port
//eg. infomations

struct dns_server{
	char host[200];
	int port;
	//int quick; //如果/etc/resolve.conf中的nameserver是url，quick = 0，否则, quick = 1
};

struct resolver_result{
	char *key; // 需要解析的host
	char **value; //解析后得到的ip列表
};

struct resolver_st{
	struct dns_server *DServer; //dns servers
	//unsigned int DSmax; //dns servers 的最大个数
	//unsigned int DSs;//dns Server的当前个数
	struct rbtree_st *addr_rbtree; //存放查询的结果，key为查询的url，value为查询的dns结果
	struct list_head name_queue;
	struct list_head address_queue;
	struct event_base *base; //reacotr 模式
};

struct resolver_st *resolver_create();
void resolver_init(struct resolver_st *resolver);
struct resolver_result *resolve_name(struct resolver_st *resolver, const char *host);
void resolver_distory(struct resolver_st *resolver);

#endif
