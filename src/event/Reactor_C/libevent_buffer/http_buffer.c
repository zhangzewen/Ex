#include "http_buffer.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

struct http_buffer *buffer_new(void)
{
	struct http_buffer *buffer;
	buffer = (struct http_buffer *)malloc(sizeof(struct http_buffer));
	return buffer;
}


void buffer_free(struct http_buffer *buffer)
{
	if (buffer->start != NULL) {
		free(buffer->start);
	}
	free(buffer);
}


static void buffer_align(struct http_buffer *buf)
{
	memmove(buf->start, buf->pos, buf->off);
	buf->pos = buf->start;
	buf->misalign = 0;
	buf->last = buf->start + buf->off;
	
}


int buffer_expand(struct http_buffer *buf, size_t datlen)
{
	size_t need = buf->misalign + buf->off + datlen;
	
	if (buf->totallen >= need) {
		return 0;
	}

	if (buf->misalign >= datlen) {
		buffer_align(buf);
	}else {
		void *newbuf;
		size_t length = buf->totallen;

		if (length < 256) {
			length = 256;
		}
		while (length < need) {
			length <<= 1;
		}

		if (buf->start != buf->pos) {
			buffer_align(buf);
		}
		if ((newbuf = realloc(buf->pos, length)) == NULL) {
			return -1;
		}

		buf->start = buf->pos = newbuf;
		buf->last = buf->start;
		buf->totallen = length;
		buf->end = buf->start + length;
	}

	return 0;
}

void buffer_drain(struct http_buffer *buf, size_t len)
{
	size_t oldoff = buf->off;
	
	if(len >= buf->off) {
		buf->pos = buf->last = buf->start;
		buf->misalign = 0;
		buf->off = 0;
		return ;
	}

	buf->pos += len;
	buf->misalign += len;
	
	buf->off -= len;
}


#define EVBUFFER_MAX_READ 4096

int buffer_read(struct http_buffer *buf, int fd, int howmuch)
{
	unsigned char *p;
	size_t oldoff = buf->off;
	int n = EVBUFFER_MAX_READ;

	if (howmuch < 0 || howmuch > n) {
		howmuch = n;
	}

	/*If we don't have FIONREAD,we might waste some space here*/

	if (buffer_expand(buf, howmuch) == -1) {
		return -1;
	}

	p = buf->last;

	n = read(fd, p, howmuch);

	if (n == -1) {
		return -1;
	}

	if (n == 0) {
		return 0;
	}

	buf->off += n;
	buf->last += n;

	return n;
}


int buffer_write(struct http_buffer *buffer, int fd)
{
	int n;
	
	n = write(fd, buffer->pos, buffer->off);

	if (n == -1) {
		return -1;
	}

	if (n == 0) {
		return 0;
	}

	buffer_drain(buffer, n);

	return n;
}

