#if 0
#ifndef __HTTP_BUFFER_H__
#define __HTTP_BUFFER_H__

#define READ_DATA_DONE 0x01
#define READ_DATA_NOT_COMPLETE 0X02
#define READ_DATA_EOF 0X03
#define READ_DATA_ERROR 0X04
#define READ_DATA_AGAIN 0x05
#define READ_DATA_BUFFER_SIZE 4096

typedef struct http_buffer_chain_st *http_buffer_chain_t;
typedef struct http_buffer_st *http_buffer_t;


struct http_buffer_st{
	struct list_head list;
	unsigned char *data;
	unsigned char *start;
	unsigned char *end;
	unsigned char *pos;
	unsigned char *last;
	
};

struct http_buffer__chain_st{
	struct list_head list;
	struct http_buffer_st buff;
};

http_buffer_t buffer_create(int MaxBufferSize);

int http_buffer_read(int fd, http_buffer_t buffer);
int http_buffer_write(int fd, http_buffer_t buffer)


#endif
#endif
#ifndef _EVBUFFER_H_INCLUDED__
#define _EVBUFFER_H_INCLUDED__

#include <stdio.h>
#include <stdarg.h>

#define EVBUFFER_READ 0X01
#define EVBUFFER_WRITE	0X02
#define EVBUFFER_EOF	0X10
#define EVBUFFER_ERROR	0X20
#define EVBUFFER_TIMEOUT	0X40

typedef struct http_buffer_st http_buffer_t;
struct http_buffer_st{
	unsigned char *pos;
	unsigned char *start;
	unsigned char *end;
	unsigned char *last;
	
	size_t misalign;
	size_t totallen;
	size_t off;
};

#define EVBUFFER_LENGTH(X) (X)->off
#define EVBUFFER_DATA(X) (X)->buffer
http_buffer_t *buffer_new(void);

void buffer_free(http_buffer_t *);

int buffer_expand(http_buffer_t *, size_t);

void buffer_drain(http_buffer_t *, size_t);

int buffer_write(http_buffer_t *, int);

int buffer_read(http_buffer_t *, int, int);

#endif
