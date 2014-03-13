#include "http_resolver.h"
#include "dns_util.h"
#include "event_base.h"
#include "dns_util.h"
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
#include <errno.h>
#include <errno.h>

int SetNoblock(int fd)
{
	int flags;
	
	if((flags = fcntl(fd, F_GETFL)) == -1) {
		return -1;
	}

	if((fcntl(fd, F_SETFL, flags | O_NONBLOCK)) == -1) {
		return -1;
	}

	return 0;
}

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
	
	if (SetNoblock(resolver->fd) != 0) {
		fprintf(stderr, "set noblocking error!\n");
		return -1;
	}

	resolver->resolve.sin_family = AF_INET;
	resolver->resolve.sin_port = htons(resolver->DServer->port);
	resolver->resolve.sin_addr.s_addr = inet_addr(resolver->DServer->host);
	fprintf(stderr, "init Reactor...\n");
	resolver->base = event_init();
	fprintf(stderr, "Reactor init done!\n");
		
	fprintf(stderr, "init net Done!\n");

	return 0;
}

void resolve_name(struct resolver_st *resolver, unsigned char *host)
{
	struct resolver_result *result = NULL;
	ssize_t nwrite = 0;
	//size_t len = 0;
	unsigned char buf[65536] = {0};
	
	int sfd = -1;

	sfd = connect(resolver->fd, (struct sockaddr *)&(resolver->resolve), sizeof(struct sockaddr_in));

	if (sfd < 0) {
		if (errno == ENETUNREACH) { //网络不可达!
			return;
		}
	}

	result = calloc(1, sizeof(struct resolver_result));
	
	if (NULL == result) {
		close(sfd); //close fd
		return;
	}


	/*
 *构造dns请求结构体
 */

	create_dns_query(host, T_A, buf, &result->question_len);

	result->key  =	NULL; 
	
	nwrite = write(resolver->fd, buf, (sizeof(struct dns_header) + (result->question_len + 1) + sizeof(struct question)));
	//nwrite = send(resolver->fd, buf, 65536, 0);
	
	if (nwrite < 0) {
		fprintf(stderr, "Can not send dns request!\n");
		return;
	}
	
	event_set(&result->ev, resolver->fd, EV_READ, parse_dns, (void *)result, NULL);
	event_add(&result->ev, NULL);
	
	
	return ;
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



