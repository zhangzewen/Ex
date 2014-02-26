#include "http_resolver.h"
#include <stdio.h>
#include <stdlib.h>

struct resolver_create()
{

	struct resolver_st *new;
	new = (struct resolver_st *)malloc(sizeof(struct resovler_st));
	if (NULL == new) {
		perror("malloc resolver_st error!\n");
		return NULL;
	}

	new->DServer = NULL;
	
	new->address_rebtree = (struct rbtree_st *)malloc(sizeof(struct rbtree_st));
	
	if (NULL == new->address_rbtree) {
		perror("malloc rbtree cache error!\n");
		free(new);
		return NULL;
	}
	
	new->base = (struct event_base *)malloc(sizeof(struct event_base));
	
	if (NULL == new->base) {
		perror("malloc event_base error!\n");
		free(new->address_rebtree);
		free(new);
		return NULL:
	}

	INIT_LIST_HEAD(&new->name_queue);	
	INIT_LIST_HEAD(&new->address_queue);
	return new;
	
}

void resolver_init(struct resolver_st *resolver)
{
	//Get_Dns_Server这个函数主要是从/etc/resolv.conf中获取nameserver，如果该/etc/resolve.conf没有配置dns服务器，就使用默认的google域名服务器：8.8.8.8:53
	resolver->DServer = Get_Dns_Server(); 
	resolver->addr_rbtree = init_rbtree();
	resolver->base = base_init();
		
}

struct resolve_result *resolve_name(struct resolver_st *resolver, const char *host)
{
	//1.查找/etc/host
	//2.查找rbtree cache,查找到了且TTL没有过期，直接返回结果，没有查找到转第3步, 如果找到了但是TTL过期了，删除记录，转到第3步 
	//3.通过DNS服务器查询，并把结果插入rbtree cache
}

void resolver_distory(struct resolver_st *resolver)
{
	DServer_destory(resolver->DServer);
	rbtree_destory(resolver->addr_rbtree);
	
}

static name_find_cache(strcut rbtree_st *root, const char *host)//先在rbtree中查找，若有且没有过期立即返回，否则通过dns查询，并把查询的结果插入到rbtree中去！
{
	
}

static char **name_fine_host(const char *host) //  从/etc/host找ip地址
{
	
}

