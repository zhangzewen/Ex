#ifndef __HTTP_BUFFER_H__
#define __HTTP_BUFFER_H__


typedef struct http_buffer_node_st *http_buffer_node_t;
typedef struct http_buffer_st *http_buffer_t;


struct http_buffer_node_st{
	struct list_head list;
	uint8_t data[256];
}http_buffer_node_t;

struct http_buffer_st{
	struct list_head list;
	int count;
	int read_ptr[2];
	int write_ptr[2];
};

http_buffer_t create_buffer(int count)
{
	http_buffer_t buffer;
	
	buffer = (http_buffer_t)malloc(sizeof(struct http_buffer_st));
	
	if(NULL == buffer){
		perror("create buffer error!\n");
		exit(1);
	}

	buffer->count = count;
	
	buffer->read_ptr[2] = {0};
	
	buffer->write_ptr[2] = {0};
		
	LIST_HEAD_INIT(&buffer->list);

	while(count) {
		http_buffer_node_t node;
		node = (http_buffer_node_t)malloc(sizeof(struct http_buffer_node_st));
		if(NULL == node) {
			continue;
		}
		
		node->data[256] = {0};
		LIST_HEAD_INIT(&node->list);
	
		list_add_tail(&node->list, &buffer->list);
		
		count--;
	}

	return buffer;
}


void destory_buffer(http_buffer_t)
{
	http_buffer_node_t ptr = NULL; 
	http_buffer_node_t tmp = NULL;
	http_buffer_t = NULL;
	
	list_for_each_entry_safe(ptr, tmp, &http_buffer_t->list, list){
		list_del(&ptr->list);
		free(ptr);
	}

	list_del(&http_buffer_t->);
	free(http_buffer_t);
	http_buffer_t = NULL;
	ptr = NULL;
	tmp = NULL;
}
	
#endif
