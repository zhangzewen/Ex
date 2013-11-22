#ifndef _EVBUFFER_H_INCLUDED__
#define _EVBUFFER_H_INCLUDED__

#include <stdio.h>
#include <stdarg.h>
#define EVBUFFER_READ 0X01
#define EVBUFFER_WRITE	0X02
#define EVBUFFER_EOF	0X10
#define EVBUFFER_ERROR	0X20
#define EVBUFFER_TIMEOUT	0X40
/*
*libevent 的缓冲是一个连续的内存区域，其处理数据的方式（写数据和读数据）更像一个队列操作方式：从后面写入，从前读出。
*
*/
struct evbuffer{
	unsigned char *buffer;//buffer指向有效的数据的内存区域
	unsigned char *orig_buffer; // 指向由malloc分配的连续内存区域
	
	size_t misalign;//表示buffer相对于orig_buffer的偏移
	size_t totallen;//拜师orig_buffer指向的内存区域的大小
	size_t off;//表示有效的数据的长度

	void (*cb)(struct evbuffer *, size_t, size_t, void *);
	void *cbarg;
};

#define EVBUFFER_LENGTH(X) (X)->off
#define EVBUFFER_DATA(X) (X)->buffer
#define EVBUFFER_INPUT(X) (X)->input
#define EVBUFFER_OUTPUT(X) (X)->output
/**
  Allocate storage for a new evbuffer.

  @return a pointer to a newly allocated evbuffer struct, or NULL if an error
          occurred
	分配一个struct evbuffer
 */
struct evbuffer *evbuffer_new(void);


/**
  Deallocate storage for an evbuffer.

  @param pointer to the evbuffer to be freed
	释放空间
 */
void evbuffer_free(struct evbuffer *);


/**
  Expands the available space in an event buffer.

  Expands the available space in the event buffer to at least datlen

  @param buf the event buffer to be expanded
  @param datlen the new minimum length requirement
  @return 0 if successful, or -1 if an error occurred
*/
int evbuffer_expand(struct evbuffer *, size_t);


/**
  Append data to the end of an evbuffer.

  @param buf the event buffer to be appended to
  @param data pointer to the beginning of the data buffer
  @param datlen the number of bytes to be copied from the data buffer
 */
int evbuffer_add(struct evbuffer *, const void *, size_t);



/**
  Read data from an event buffer and drain the bytes read.

  @param buf the event buffer to be read from
  @param data the destination buffer to store the result
  @param datlen the maximum size of the destination buffer
  @return the number of bytes read
 */
int evbuffer_remove(struct evbuffer *, void *, size_t);


/**
 * Read a single line from an event buffer.
 *
 * Reads a line terminated by either '\r\n', '\n\r' or '\r' or '\n'.
 * The returned buffer needs to be freed by the caller.
 *
 * @param buffer the evbuffer to read from
 * @return pointer to a single line, or NULL if an error occurred
 */
char *evbuffer_readline(struct evbuffer *);


/** Used to tell evbuffer_readln what kind of line-ending to look for.
 */
enum evbuffer_eol_style {
  /** Any sequence of CR and LF characters is acceptable as an EOL. */
  EVBUFFER_EOL_ANY,
  /** An EOL is an LF, optionally preceded by a CR.  This style is
   * most useful for implementing text-based internet protocols. */
  EVBUFFER_EOL_CRLF,
  /** An EOL is a CR followed by an LF. */
  EVBUFFER_EOL_CRLF_STRICT,
  /** An EOL is a LF. */
        EVBUFFER_EOL_LF
};

/**
 * Read a single line from an event buffer.
 *
 * Reads a line terminated by an EOL as determined by the evbuffer_eol_style
 * argument.  Returns a newly allocated nul-terminated string; the caller must
 * free the returned value.  The EOL is not included in the returned string.
 *
 * @param buffer the evbuffer to read from
 * @param n_read_out if non-NULL, points to a size_t that is set to the
 *       number of characters in the returned string.  This is useful for
 *       strings that can contain NUL characters.
 * @param eol_style the style of line-ending to use.
 * @return pointer to a single line, or NULL if an error occurred
 */
char *evbuffer_readln(struct evbuffer *buffer, size_t *n_read_out,
    enum evbuffer_eol_style eol_style);


/**
  Move data from one evbuffer into another evbuffer.

  This is a destructive add.  The data from one buffer moves into
  the other buffer. The destination buffer is expanded as needed.

  @param outbuf the output buffer
  @param inbuf the input buffer
  @return 0 if successful, or -1 if an error occurred
 */
int evbuffer_add_buffer(struct evbuffer *, struct evbuffer *);

/**
  Append a formatted string to the end of an evbuffer.

  @param buf the evbuffer that will be appended to
  @param fmt a format string
  @param ... arguments that will be passed to printf(3)
  @return The number of bytes added if successful, or -1 if an error occurred.
 */
int evbuffer_add_printf(struct evbuffer *, const char *fmt, ...)
#ifdef __GNUC__
  __attribute__((format(printf, 2, 3)))
#endif
;


/**
  Append a va_list formatted string to the end of an evbuffer.

  @param buf the evbuffer that will be appended to
  @param fmt a format string
  @param ap a varargs va_list argument array that will be passed to vprintf(3)
  @return The number of bytes added if successful, or -1 if an error occurred.
 */
int evbuffer_add_vprintf(struct evbuffer *, const char *fmt, va_list ap);


/**
  Remove a specified number of bytes data from the beginning of an evbuffer.

  @param buf the evbuffer to be drained
  @param len the number of bytes to drain from the beginning of the buffer
 */
void evbuffer_drain(struct evbuffer *, size_t);


/**
  Write the contents of an evbuffer to a file descriptor.

  The evbuffer will be drained after the bytes have been successfully written.

  @param buffer the evbuffer to be written and drained
  @param fd the file descriptor to be written to
  @return the number of bytes written, or -1 if an error occurred
  @see evbuffer_read()
 */
int evbuffer_write(struct evbuffer *, int);


/**
  Read from a file descriptor and store the result in an evbuffer.

  @param buf the evbuffer to store the result
  @param fd the file descriptor to read from
  @param howmuch the number of bytes to be read
  @return the number of bytes read, or -1 if an error occurred
  @see evbuffer_write()
 */
int evbuffer_read(struct evbuffer *, int, int);
/**
  Find a string within an evbuffer.

  @param buffer the evbuffer to be searched
  @param what the string to be searched for
  @param len the length of the search string
  @return a pointer to the beginning of the search string, or NULL if the search failed.
 */
unsigned char  *evbuffer_find(struct evbuffer *, const unsigned char *, size_t);

/**
  Set a callback to invoke when the evbuffer is modified.

  @param buffer the evbuffer to be monitored
  @param cb the callback function to invoke when the evbuffer is modified
  @param cbarg an argument to be provided to the callback function
 */
void evbuffer_setcb(struct evbuffer *, void (*)(struct evbuffer *, size_t, size_t, void *), void *);

/*
 * Marshaling tagged data - We assume that all tags are inserted in their
 * numeric order - so that unknown tags will always be higher than the
 * known ones - and we can just ignore the end of an event buffer.
 */

int evutil_vsnprintf(char *buf, size_t buflen, const char *format, va_list ap);

#endif
