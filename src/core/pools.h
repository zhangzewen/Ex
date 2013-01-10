#ifndef _HTTP_POOLS_H_INCLUDED
#define -HTTP_POOLS_H_INCLUDED

typedef struct _memory_pool
{
	unsigned int block_count;
	unsigned int block_count_step;
	unsigned int block_size;
	unsigned int free_count;
	memory_pool *free_head;
	memory_pool *free_tail;
	memory_pool *used_head;
	memory_pool *used_tail;
	char *pBlock_memory_head;
	char *pDate_memory_head;
	struct list list_head;
}memory_pool;

int pool_init();
int pool_destroy();
void *pool_alloc();
int pool_free();
static int pool_block();
#endif
