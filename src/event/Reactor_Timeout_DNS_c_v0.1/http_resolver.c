#define _GNU_SOURCE
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

static unsigned int dnserver_index;

int get_dns_server(struct dns_server *dns, int *count)
{
	FILE * fp;
	char * line = NULL;
	size_t len = 0;
	ssize_t read;
	int current = 0;
	
	fp = fopen(RESOLVER_CONFIG_FILE, "r");
	if (fp == NULL) {
		return -1;
	}

	while ((read = getline(&line, &len, fp)) != -1) {
		printf("%s", line);
		
		if (line[0] == '#') {
			continue;
		}

		if (strncmp(line, "nameserver", 10) == 0) {
			if (current >= MAX_DNS_SERVERS) {
				break;
			}
			char host[DEFAULT_HOST_LENGTH] = {0};
			sscanf(line, "%*s%*[ \t]%s%*s", host);
			strcpy(dns[current].host, host);
			dns[current].port = 53;
			current++;
		} 	
		
	}

	if (line) {
		free(line);
	}

	fclose(fp);
	*count = current;

	return 0;
}


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

	//new->DServer = NULL;

	
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


int resolver_init(struct resolver_st *resolve)
{
	//Get_Dns_Server这个函数主要是从/etc/resolv.conf中获取nameserver，如果该/etc/resolve.conf没有配置dns服务器，就使用默认的google域名服务器：8.8.8.8:53
	//resolver->DServer = Get_Dns_Server(); 
//=========int dns server====================
	fprintf(stderr, "init dns server ....\n");	
	memset(resolve->DServer, 0, (sizeof(struct dns_server)* MAX_DNS_SERVERS));
	if (get_dns_server(resolve->DServer, &resolve->count) == -1) {
		fprintf(stderr, "get dns servers error!\n");
		return -1;
	}
	fprintf(stderr, "Done!\n");
//=======init rbtree===============
	fprintf(stderr, "init dns cache");
	rbtree_init(resolve->addr_rbtree);
	fprintf(stderr, "Done!\n");
	
//=======init  reactor============
	fprintf(stderr, "init Reactor...\n");
	resolve->base = event_init();
	fprintf(stderr, "Reactor init done!\n");
		
	return 0;
}

	//1.查找/etc/host
	//2.查找rbtree cache,查找到了且TTL没有过期，直接返回结果，没有查找到转第3步, 如果找到了但是TTL过期了，删除记录，转到第3步 
	//3.通过DNS服务器查询，并把结果插入rbtree cache

void resolve_name(struct resolver_st *resolver, unsigned char *host)
{
	struct resolver_result *result = NULL;
	struct dns_server *dns = NULL;
	struct sockaddr_in remote;
	ssize_t nwrite = 0;
	int fd = -1;
	int robin = -1;
	unsigned char buf[65536] = {0};
	
	int sfd = -1;

	result = calloc(1, sizeof(struct resolver_result));
	
	if (NULL == result) {
		close(sfd); //close fd
		return;
	}

	fd = socket(AF_INET, SOCK_DGRAM, 0);

	if (fd < 0) {
		fprintf(stderr,"socket error!=\n");
		return;
	}

	if (SetNoblock(fd) != 0) {
		fprintf(stderr, "set noblocking error!\n");
		return ;
	}
	// just round robin
	robin = (dnserver_index + 1) % resolver->count;
	dnserver_index = robin;

	dns = resolver->DServer + robin;

	remote.sin_family = AF_INET;
	remote.sin_port = htons(dns->port);
	inet_pton(AF_INET, dns->host, &remote.sin_addr);


	sfd = connect(fd, (struct sockaddr *)&remote, sizeof(struct sockaddr_in));

	if (sfd < 0) {
		if (errno == ENETUNREACH) { //网络不可达!
			return;
		}
	}

	/*
 *构造dns请求结构体
 */

	create_dns_query(host, T_A, buf, &result->question_len);

	result->key  =	NULL; 
	nwrite = write(fd, buf, (sizeof(struct dns_header) + (result->question_len + 1) + sizeof(struct question)));
	//fprintf(stderr, "buff(len = %d):%s", (int)nwrite, buf );	
	//nwrite = send(resolver->fd, buf, 65536, 0);
	
	if (nwrite < 0) {
		fprintf(stderr, "Can not send dns request!\n");
		return;
	}
	
	event_set(&result->ev, fd, EV_READ, parse_dns, (void *)result, NULL);
	event_add(&result->ev, NULL);
	
	
	return ;
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



