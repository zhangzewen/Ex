#ifndef _RBTREE_H_INCLUDED__
#define _RBTREE_H_INCLUDED__

#include <stdint.h>
typedef enum color_t{
	RED = 0,
	BLACK = 1
}color_t;

typedef struct rbtree_node_st rbtree_node_t;

typedef struct rbtree_st rbtree_t;

struct rbtree_node_st{
	uintptr_t key;
	void *data;
	color_t color;
	struct rbtree_node_st *left, *right, *parent;
};

struct rbtree_st{
	struct rbtree_node_st *root;
	struct rbtree_node_st* (*insert)(uintptr_t key, void *data, struct rbtree_node_st *root);
	struct rbtree_node_st* (*search)(uintptr_t key, struct rbtree_node_st *root);
	int (*erase)(uintptr_t key, struct rbtree_node_st *root);
	void* (*min)(struct rbtree_node_st *root);
	int (*empty)(struct rbtree_node_st *root);

};

void rbtree_init(rbtree_t *tree);

struct rbtree_node_st *rb_insert(uintptr_t key, void* data, struct rbtree_node_st *root);
struct rbtree_node_st *rb_search(uintptr_t key, struct rbtree_node_st *root);
struct rbtree_node_st *rb_erase(uintptr_t key, struct rbtree_node_st *root);
void *rb_min(struct rbtree_node_st *root);
int rbtree_empty(rbtree_t *tree);


#endif
