#if 0
#ifndef __HTTP_BUFFER_H__
#define __HTTP_BUFFER_H__

/*
*start							pos								last		  		end
*|-------------------|-----------------|--------------|
*         ^									^									^
*         |									|									|
*					|									|									|	
*		已经处理部分					未处理部分			可读入部分
*
*start ------->last  这个部分是读入的部分
*start--------->pos   这个部分标志已经处理了这部分读入的数据，这部分可以进行擦除操作（数据覆盖）
*pos----------->last  这部分的数据还没有来的急处理
*last----------end   这部分还是空白，没有读入数据
*/


http_buffer_t create_buffer(int MaxBufferSize)
{
	http_buffer_t buffer;
	
	buffer = (http_buffer_t)malloc(sizeof(struct http_buffer_st));
	
	if(NULL == buffer) {
		return NULL;
	}

	buffer->data = (unsigned char *)malloc(MaxBufferSize);
	
	if(NULL == buffer->data) {
		free(buffer);
		return NULL;
	}

	memset(buffer->data, 0, MaxBufferSize);

	buffer->start = buffer->data;
	buffer->end = buffer->data + MaxBufferSize;
	buffer->pos = buffer->data;
	buffer->last = buffer->data;
	
	return buffer;
}


void destory_buffer(http_buffer_t buffer)
{
	if(NULL == buffer) {
		return;
	}
	
	if(buffer->data) {
		free(buffer->data);
	}
	
	if(!list_empty(&buffer->list)){
		list_del(&buffer->list);
	}

	free(buffer);
	buffer = NULL;

	return;
}


/*
*
*http_buffer_read for http request parase
*/


/*
*
*1,短连接
*2,长连接
*
*/
int http_buffer_read(int fd, http_buffer_t buffer, int op)
{
	int n = 0;
	int nread = 0;
	
	n = buffer->last - buffer->pos;
#if 0
	
	if(n > 0) {  //就是以读取但未处理的部分还有，先处理未处理的部分
		return n;
	}
#endif
	if(n && buffer->last == buffer->end) {
		if(buffer->pos == buffer->start) { //在此时的情况下buffer都被写满了，但是还是没有从这写满的
			return READ_DATA_ERROR;
		}
		
		if(op == READ_DATA_DONE) { //解析的数据完整,当在epoll水平触发的时候，可能还会有数据需要接受，但是要先处理数据
			return op;
		}
		
		if(op == READ_DATA_NOT_COMPLETE) {
			memcopy(buffer->start, buffer->pos, n); // 解析的数据目前还不完整 ,吧数据copy到buffer的开始
			buffer->pos = buffer->start;
			buffer->last  = buffer->pos + n;
			nread = read(fd, buffer->last, buffer->end - buffer->last);
			// do with nread;
			if(nread < 0) {
				if(errno == EAGAIN) {
					return READ_DATA_AGAIN;
				}else {
					return READ_DATA_ERROR;
				}
			}

			if(nread == 0){
				return READ_DATA_EOF;
			}

			buffer->last = buffer->last + nread;
			return nread;
		}
		
	}

	if(n && buffer->last != buffer->end) {
		if(op == READ_DATA_NOT_COMPLETE) {
			nread = read(fd, buffer->last, buffer->end - buffer->last);
			//do with nread;
			if(nread < 0) {
				if(errno == EAGAIN) {
					return READ_DATA_AGAIN;
				}else {
					return READ_DATA_ERROR;
				}
			}

			if(nread == 0){
				return READ_DATA_EOF;
			}
			
			buffer->last = buffer->last + nread;
			return nread;
		}
		
		if(op == READ_DATA_DONE){
			return op;
		}
	}

	if(n == 0 && buffer->pos == buffer->start) {
		if(op == READ_DATA_NOT_COMPLETE) {
			nread = read(fd ,buffer->start, buffer->end - buffer->start);
			//do with nread
			if(nread < 0) {
				if(errno == EAGAIN) {
					return READ_DATA_AGAIN;
				}else {
					return READ_DATA_ERROR;
				}
			}

			if(nread == 0){
				return READ_DATA_EOF;
			}
			
			buffer->last = buffer->last + nread;
			return nread;
		}
		
		if(op == READ_DATA_DONE) {
			return  READ_DATA_DONE;
		}
		
	}

	if(n == 0 && buffer->pos == buffer->end) {
		buffer->pos = buffer->start;
		buffer->last = buffer->pos;
		if(op == READ_DATA_NOT_COMPLETE) {
			nread = read(fd, buffer->last, bufer->end - buffer->last);
			//do with nread
			if(nread < 0) {
				if(errno == EAGAIN) {
					return READ_DATA_AGAIN;
				}else{
					return READ_DATA_ERROR;
				}
			}

			if(nread == 0) {
				return READ_DATA_EOF;
			}

			buffer->last = buffer->last + nread;
			return nread;
		}
		
		if(op == READ_DATA_DONE) {
			return op;
		}
	}

	return READ_DATA_ERROR;	

}

int http_buffer_read(int fd, http_buffer_t buffer)
{
	
}
	
#endif
#endif
#include "http_buffer.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

http_buffer_t *buffer_new(void)
{
	http_buffer_t *buffer;
	buffer = (http_buffer_t *)malloc(sizeof(http_buffer_t));
	return buffer;
}


void buffer_free(http_buffer_t *buffer)
{
	if (buffer->start != NULL) {
		free(buffer->start);
	}
	free(buffer);
}


static void buffer_align(http_buffer_t *buf)
{
	memmove(buf->start, buf->pos, buf->off);
	buf->pos = buf->start;
	buf->misalign = 0;
	buf->last = buf->start + buf->off;
	
}


int buffer_expand(http_buffer_t *buf, size_t datlen)
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

void buffer_drain(http_buffer_t *buf, size_t len)
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

int buffer_read(http_buffer_t *buf, int fd, int howmuch)
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


int buffer_write(http_buffer_t *buffer, int fd)
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

