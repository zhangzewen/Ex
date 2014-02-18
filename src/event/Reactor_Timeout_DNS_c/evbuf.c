#include "evbuf.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

struct evbuffer * evbuffer_new(void)
{
	struct evbuffer *buffer;
	buffer = (struct evbuffer *)malloc(sizeof(struct evbuffer));
	return buffer;
}


void evbuffer_free(struct evbuffer *buffer)
{
	if (buffer->orig_buffer != NULL) {
		free(buffer->orig_buffer);
	}
	free(buffer);
}


#define SWAP(x, y) do {\
	(x)->buffer = (y)->buffer; \
	(x)->orig_buffer = (y)->orig_buffer; \
	(x)->misalign = (y)->misalign; \
	(x)->totallen = (y)->totallen;	\
	(x)->off = (y)->off;	\
}while (0)

int evbuffer_add_buffer(struct evbuffer *outbuf, struct evbuffer *inbuf)
{
	int res;
	
	if(outbuf->off == 0) {
		struct evbuffer tmp;
		size_t oldoff = inbuf->off;
		
		SWAP(&tmp, outbuf);
		SWAP(outbuf, inbuf);
		SWAP(inbuf, &tmp);
		
		if (inbuf->off != oldoff && inbuf->cb != NULL) {
			(*inbuf->cb)(inbuf, oldoff, inbuf->off, inbuf->cbarg);
		}
		
		if (oldoff && outbuf->cb != NULL) {
			(*outbuf->cb)(outbuf, 0, oldoff, outbuf->cbarg);
		}
		
		return 0;
	}
	
	res = evbuffer_add(outbuf, inbuf->buffer, inbuf->off);

	if (res == 0) {
		evbuffer_drain(inbuf, inbuf->off);
	}
	return res;
}


int evbuffer_add_vprintf(struct evbuffer *buf, const char *fmt, va_list ap)
{
	char *buffer;
	size_t space;
	size_t oldoff = buf->off;
	int sz;
	va_list aq;

	evbuffer_expand(buf, 64);
	
	for(;;) {
		size_t used = buf->misalign + buf->off;
		buffer = (char *)buf->buffer + buf->off;
		assert(buf->totallen >= used);
		space = buf->totallen - used;

#ifndef va_copy
#define va_copy(dst, src) memcpy(&(dst), &(src), sizeof(va_list))
#endif
		
		va_copy(aq, ap);
		sz = evutil_vsnprintf(buffer, space, fmt, aq);

		va_end(aq);

		if (sz < 0) {
			return -1;
		}

		if ((size_t)sz < space) {
			buf->off += sz;
			if (buf->cb != NULL) {
				(*buf->cb)(buf, oldoff, buf->off, buf->cbarg);
			}
			return sz;
		}

		if (evbuffer_expand(buf, sz + 1) == -1) {
			return -1;
		}
	}
}

int evutil_vsnprintf(char *buf, size_t buflen, const char *format, va_list ap)
{
	int r = vsnprintf(buf, buflen, format, ap);
	buf[buflen - 1] = '\0';
	return r;
}

int evbuffer_add_printf(struct evbuffer *buf, const char *fmt, ...)
{
	int res = -1;
	va_list ap;
	
	va_start(ap, fmt);

	res = evbuffer_add_vprintf(buf, fmt, ap);
	va_end(ap);

	return res;
}

int evbuffer_remove(struct evbuffer *buf, void *data, size_t datlen)
{
	size_t nread = datlen;
	if (nread >= buf->off) {
		nread = buf->off;
	}

	memcpy(data, buf->buffer, nread);
	evbuffer_drain(buf, nread);
	return nread;
}

/*
	*Reads a line terminated by either '\r\n', '\n\r' or '\r' or '\n'.
	*The returned buffer needs to be freed by the called.
	*/


char *evbuffer_readline(struct evbuffer *buffer)
{
	unsigned char *data = EVBUFFER_DATA(buffer);
	size_t len = EVBUFFER_LENGTH(buffer);
	char *line;
	unsigned int i;
	
	for(i = 0; i < len; i++) {
		if(data[i] == '\r' || data[i] == '\n') {
			break;
		}
	}
		
	if(i == len) {
		return NULL;
	}
	
	if ((line = malloc(i + 1)) == NULL) {
		fprintf(stderr, "%s: out of memory\n", __func__);
		return NULL;
	}

	memcpy(line, data, i);
	line[i] = '\0';
	
	/*
	*Some protocols terminate a line with '\r\n', so check for that, too
	*/

	if (i < len - 1) {
		char fch = data[i];
		char sch = data[i + 1];
		/*Drain one more characher if needed*/
		if ((sch == '\r' || sch == '\n') && sch != fch) {
			i += 1;
		}
	}
	
	evbuffer_drain(buffer, i + 1);
	return line;
}


char *evbuffer_readln(struct evbuffer *buffer, size_t *n_read_out, enum evbuffer_eol_style eol_style)
{
	unsigned char *data = EVBUFFER_DATA(buffer);
	unsigned char *start_of_eol;
	unsigned char *end_of_eol;
	size_t len = EVBUFFER_LENGTH(buffer);

	char *line;
	unsigned int i;
	unsigned int n_to_copy;
	unsigned int n_to_drain;

	if(n_read_out) {
		*n_read_out = 0;
	}

	//depending on eol_style, set start_of_eol to the first character
	//in the newline, and end_of_eol to one after the last character.
	
	switch(eol_style) {
		case EVBUFFER_EOL_ANY:
			for(i = 0; i < len; i++) {
				if (data[i] == '\r' || data[i] == '\n') {
					break;
				}
			}
			if (i == len) {
				return NULL;
			}
			start_of_eol = data + i;
			++i;
			for (; i < len; i++) {
				if (data[i] != '\r' && data[i] != '\n') {
					break;
				}
			}
			end_of_eol = data + i;
			break;
		case EVBUFFER_EOL_CRLF:
			end_of_eol = memchr(data, '\n', len);
			if(!end_of_eol) {
				return NULL;
			}

			if (end_of_eol > data && *(end_of_eol - 1) == '\r') {
				start_of_eol = end_of_eol - 1;
			}else {
				start_of_eol = end_of_eol;
			}

			end_of_eol++; /*point to one after the LF.*/
			break;
		case EVBUFFER_EOL_CRLF_STRICT: {
			unsigned char *cp = data;
			while ((cp = memchr(cp, '\r', len - (cp - data)))) {
				if (cp < data + len - 1 && *(cp + 1) == '\n') {
					break;
				}
				if (++cp >= data + len) {
					cp = NULL;
					break;
				}
			}
			
			if (!cp) {
				return NULL;
			}
			start_of_eol = cp;
			end_of_eol = cp + 2;
			break;
		}
		case EVBUFFER_EOL_LF:
			start_of_eol = memchr(data, '\n', len);
			if (!start_of_eol) {
				return NULL;
			}
			end_of_eol = start_of_eol + 1;
			break;
		default:
			return NULL;
	}

	n_to_copy = start_of_eol - data;
	n_to_drain = end_of_eol - data;
	
	if ((line = malloc(n_to_copy + 1)) == NULL) {
		fprintf(stderr, "%s: out of memory\n", __func__);
		return NULL;
	}

	memcpy(line, data, n_to_copy);
	line[n_to_copy] = '\0';

	evbuffer_drain(buffer, n_to_drain);
	if (n_read_out) {
		*n_read_out = (size_t)n_to_copy;
	}

	return line;
}


/*Adds data to an event buffer*/

static void evbuffer_align(struct evbuffer *buf)
{
	memmove(buf->orig_buffer, buf->buffer, buf->off);
	buf->buffer = buf->orig_buffer;
	buf->misalign = 0;
}

/*Expands the available space in the event buffer to at least datlen*/

int evbuffer_expand(struct evbuffer *buf, size_t datlen)
{
	size_t need = buf->misalign + buf->off + datlen;
	
	/*If we can fit all the data, the we don't have to do anything*/
	if (buf->totallen >= need) {
		return 0;
	}

	//If the misalignment fulfills out data needs, we just force an 
	//alignment to happen, Afterwards ,we have enough space.
	if (buf->misalign >= datlen) {
		evbuffer_align(buf);
	}else {
		void *newbuf;
		size_t length = buf->totallen;

		if (length < 256) {
			length = 16;
		}
		while (length < need) {
			length <<= 1;
		}

		if (buf->orig_buffer != buf->buffer) {
			evbuffer_align(buf);
		}
		if ((newbuf = realloc(buf->buffer, length)) == NULL) {
			return -1;
		}

		buf->orig_buffer = buf->buffer = newbuf;
		buf->totallen = length;
	}

	return 0;
}


int evbuffer_add(struct evbuffer *buf, const void *data, size_t datlen)
{
	size_t need = buf->misalign + buf->off + datlen;
	size_t oldoff = buf->off;
	
	if (buf->totallen < need) {
		if (evbuffer_expand(buf, datlen) == -1) {
			return -1;
		}
	}

	memcpy(buf->buffer + buf->off, data, datlen);
	buf->off += datlen;
	if (datlen && buf->cb != NULL) {
		(*buf->cb)(buf, oldoff, buf->off, buf->cbarg);
	}
	return 0;
}

void evbuffer_drain(struct evbuffer *buf, size_t len)
{
	size_t oldoff = buf->off;
	
	if(len >= buf->off) {
		buf->off = 0;
		buf->buffer = buf->orig_buffer;
		buf->misalign = 0;
		goto done;
	}

	buf->buffer += len;
	buf->misalign += len;
	
	buf->off -= len;
done:
	// Tell someone about changes in this buffer
	if (buf->off != oldoff &&  buf->cb != NULL) {
		(*buf->cb)(buf, oldoff, buf->off, buf->cbarg);
	}
}


/*
 *Reads data from a file descriptor into a buffer.
 */

#define EVBUFFER_MAX_READ 4096

int evbuffer_read(struct evbuffer *buf, int fd, int howmuch)
{
	unsigned char *p;
	size_t oldoff = buf->off;
	int n = EVBUFFER_MAX_READ;

	if (howmuch < 0 || howmuch > n) {
		howmuch = n;
	}

	/*If we don't have FIONREAD,we might waste some space here*/

	if (evbuffer_expand(buf, howmuch) == -1) {
		return -1;
	}

	p = buf->buffer + buf->off;

	n = read(fd, p, howmuch);

	if (n == -1) {
		return -1;
	}

	if (n == 0) {
		return 0;
	}

	buf->off += n;

	/*Tell somone abot changes in this buffer*/

	if (buf->off != oldoff && buf->cb != NULL) {
		(*buf->cb)(buf, oldoff, buf->off, buf->cbarg);
	}

	return n;
}


int evbuffer_write(struct evbuffer *buffer, int fd)
{
	int n;
	
	n = write(fd, buffer->buffer, buffer->off);

	if (n == -1) {
		return -1;
	}

	if (n == 0) {
		return 0;
	}

	evbuffer_drain(buffer, n);

	return n;
}

unsigned char *evbuffer_find(struct evbuffer *buffer, const unsigned char *what, size_t len)
{
	unsigned char *search = buffer->buffer;
	unsigned char *end = search + buffer->off;

	unsigned char *p;
	
	while(search < end && (p = memchr(search, *what, end - search)) != NULL) {
		if (p + len > end) {
			break;
		}

		if (memcmp(p, what, len) == 0) {
			return p;
		}

		search = p + 1;
	}
	return NULL;
}

void evbuffer_setcb(struct evbuffer *buffer, void (*cb)(struct evbuffer *, size_t, size_t, void *), void *cbarg)
{
	buffer->cb = cb;
	buffer->cbarg = cbarg;
}
