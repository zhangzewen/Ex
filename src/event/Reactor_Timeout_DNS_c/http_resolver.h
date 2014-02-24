#ifndef _HTTP_RESOLVE_H_INCLUDED__
#define _HTTP_RESOLVE_H_INCLUDED__

//struct dns_server record
//the dns server ip,port
//eg. infomations
struct dns_server{
	char host[200];
	char dot_address[128];
	int default_port;
	int port;
};

struct resolver_result{
	char *key; // 需要解析的host
	char **value; //解析后得到的ip列表
};


typedef struct resolver_st resolver;

struct resolver_st{
	struct dns_server *DServer; //dns servers
	struct rbtree_st *addr_rbtree; //存放查询的结果，key为查询的url，value为查询的dns结果
};


resolver_init();
resolve_name();
resolver_distory();


#endif
