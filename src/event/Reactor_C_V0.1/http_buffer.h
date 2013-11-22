#ifndef _HTTP_BUFFER_H_INCLUDED__
#define _HTTP_BUFFER_H_INCLUDED__

#include <stdio.h>
#include <stdarg.h>
typedef struct http_buffer_st http_buffer_t;
struct http_buffer_st{
	char *pos;
	char *start;
	char *end;
	char *last;
	
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
