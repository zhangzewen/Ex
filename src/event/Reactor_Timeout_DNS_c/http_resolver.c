#include "http_resolver.h"
#include "event_base.h"
#include "event.h"
#include "RBTree.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/types.h>

struct resolver_st* resolver_create()
{

	struct resolver_st *new;
	new = (struct resolver_st *)malloc(sizeof(struct resolver_st));
	if (NULL == new) {
		perror("malloc resolver_st error!\n");
		return NULL;
	}

	new->DServer = NULL;
	
	new->addr_rbtree = (struct rbtree_st *)malloc(sizeof(struct rbtree_st));
	
	if (NULL == new->addr_rbtree) {
		perror("malloc rbtree cache error!\n");
		free(new);
		return NULL;
	}
	
	new->base = event_init();
	
	if (NULL == new->base) {
		perror("malloc event_base error!\n");
		free(new->addr_rbtree);
		free(new);
		return NULL;
	}

	return new;
}

int resolver_init(struct resolver_st *resolver)
{
	//Get_Dns_Server这个函数主要是从/etc/resolv.conf中获取nameserver，如果该/etc/resolve.conf没有配置dns服务器，就使用默认的google域名服务器：8.8.8.8:53
	//resolver->DServer = Get_Dns_Server(); 
	fprintf(stderr, "init dns server ....\n");	
	resolver->DServer = (struct dns_server *)malloc(sizeof(struct dns_server));
	
	if (NULL == resolver->DServer) {
		return -1;
	}

	strcpy(resolver->DServer->host, "8.8.8.8");
	resolver->DServer->port = 53;
	fprintf(stderr, "init dns server Done!\n");
	fprintf(stderr, "init dns cache");
	rbtree_init(resolver->addr_rbtree);
	fprintf(stderr, "Done!\n");
	
	fprintf(stderr, "init net....\n");
	
	resolver->fd = socket(AF_INET, SOCK_DGRAM, 0);
	
	if (resolver->fd < 0) {
		fprintf(stderr, "init net error!\n");
		return -1;
	}

	resolver->local.sin_family = AF_INET;
	resolver->local.sin_port = htons(resolver->DServer->port);
	resolver->local.sin_addr.s_addr = inet_addr(resolver->DServer->host);
		
	fprintf(stderr, "init net Done!\n");

	return 0;
}

struct resolve_result *resolve_name(struct resolver_st *resolver, const char *host)
{
	struct resolve_result *result = NULL;
	return result;
	//1.查找/etc/host
	//2.查找rbtree cache,查找到了且TTL没有过期，直接返回结果，没有查找到转第3步, 如果找到了但是TTL过期了，删除记录，转到第3步 
	//3.通过DNS服务器查询，并把结果插入rbtree cache
}

void resolver_distory(struct resolver_st *resolver)
{

}

#if 0
static name_find_cache(strcut rbtree_st *root, const char *host)//先在rbtree中查找，若有且没有过期立即返回，否则通过dns查询，并把查询的结果插入到rbtree中去！
{
	
}

static char **name_fine_host(const char *host) //  从/etc/host找ip地址
{
	
}
#endif

