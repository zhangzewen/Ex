#ifndef _RBTREE_H_INCLUDED__
#define _RBTREE_H_INCLUDED__

typedef int key_t;
typedef int data_t;

typedef enum color_t{
	RED = 0,
	BLACK = 1
}color_t;

typedef struct rb_node_t{
	struct rb_node_t *left;
	struct rb_node_t *right;
	struct rb_node_t *parent;
	uintptr_t key; //计算出来的时间

	void *ptr;//指向在事件队列的event
	color_t color;
}rb_node_t;

typedef struct rb_tree{
	rb_node_t *root;
	rb_node_t* (*insert)(void *data, rb_node_t *root);
	rb_node_t* (*search)(void *data, rb_node_t *root);
	int (*erase)(void *data, rb_node_t *root);
	void* (*min)(rb_node_t *root);
	int (*empty)(rb_node_t *root);

} rb_tree_t;

void rb_tree_create(struct rb_tree *tree);

rb_node_t *rb_insert(data_t data, rb_node_t *root);
rb_node_t *rb_search(rb_node_t *root);
int *rb_erase(void *data, rb_node_t *root);
void *rb_min(rb_node_t *root);
int rb_tree_empty(rb_node_t *root);





#endif
