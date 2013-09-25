#ifndef _EVBUFFER_H_INCLUDED__
#define _EVBUFFER_H_INCLUDED__

#include <stdio.h>
#include <stdarg.h>

#define EVBUFFER_READ 0X01
#define EVBUFFER_WRITE	0X02
#define EVBUFFER_EOF	0X10
#define EVBUFFER_ERROR	0X20
#define EVBUFFER_TIMEOUT	0X40

struct http_buffer{
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
struct http_buffer *buffer_new(void);

void buffer_free(struct http_buffer *);

int buffer_expand(struct http_buffer *, size_t);

void buffer_drain(struct http_buffer *, size_t);

int buffer_write(struct http_buffer *, int);

int buffer_read(struct http_buffer *, int, int);

#endif
