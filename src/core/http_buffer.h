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
