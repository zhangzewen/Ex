#ifndef _HTTP_REQUEST_H_INCLUDED_
#define _HTTP_REQUEST_H_INCLUDED_

#include "evbuf.h"

typedef struct http_request_st http_request_t;
typedef struct http_connection_st http_connection_t;

struct http_connection_st{
	struct event *read; //accept后读事件
	struct event *write;//这个暂时做NULL处理
	struct evbuffer *buffer;
	int fd;
	
	http_request_t *r;
};

struct http_request_st{
	http_connection_t *c;
	
	char *method_start;
	char *method_end;

	char *path_start;
	char *path_end;

	char *version_start;
	char *version_end;

	char *key_start;
	char *key_end;

	char *value_start;
	char *value_end;
	char *tmp;
	
	int parse_state;

	
};

struct http_peer_request_st {
	struct http_request_st *dsrequest;
};

http_request_t* init_request();

http_connection_t* init_connection();

#if 0
void http_init_connection(http_connection_t *c); //初始化connection

void http_init_request(http_request_t *r); //初始化 request

void http_free_connection(http_connection_t *c); //释放connection

void http_free_request(http_request_t *r); //释放request
#endif

#endif


