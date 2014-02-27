#include "RBTree.h"
#include <stdio.h>
#include <stdlib.h>
#include "event_base.h"

static rbtree_node_t *rb_new_node(uintptr_t key, void* data)
{
	rbtree_node_t *node = (struct rbtree_node_st *)malloc(sizeof(struct rbtree_node_st));
	
	struct event *ev = (struct event *)data;
	if (!node) {
		printf("malloc error!\n");
		exit(-1);
	}
	
	node->key = key;
	node->data = data;
	node->name = ev->name;
	return node;
}

static rbtree_node_t *rb_insert_rebalance(rbtree_node_t *node, rbtree_node_t *root);
static rbtree_node_t *rb_erase_rebalance(rbtree_node_t *node, rbtree_node_t *parent, rbtree_node_t *root);
static rbtree_node_t *rb_rotate_left(rbtree_node_t *node, rbtree_node_t *root)
{
	rbtree_node_t *right = node->right;

	if ((node->right = right->left)) {
		right->left->parent = node;
	}
	
	right->left = node;
	
	if ((right->parent = node->parent)) {
		if (node == node->parent->right) {
			node->parent->right = right;
		}else {
			node->parent->left = right;	
		}
	}else {
		root = right;
	}
	
	node->parent = right;
	
	return root;
}


static rbtree_node_t *rb_rotate_right(rbtree_node_t *node, rbtree_node_t *root)
{
	rbtree_node_t *left = node->left;
	
	if ((node->left = left->right)) {
		left->right->parent = node;
	}

	left->right = node;
	
	if ((left->parent = node->parent)) {
		if (node == node->parent->right) {
			node->parent->right = left;
		}else {
			node->parent->left = left;
		}
	}else {
		root = left;
	}

	node->parent = left;
	
	return root;
}


static rbtree_node_t *rb_search_auxiliary(uintptr_t key, rbtree_node_t *root, rbtree_node_t **save)
{
	rbtree_node_t *node = root;
	rbtree_node_t *parent = NULL;
	int ret = 0;
	
	while(node) {
		parent = node;
		ret = node->key - key;
		
		if(0 < ret) {
			node = node->left;
		} else if (0 > ret) {
			node = node->right;
		}else {
			return node;
		}
	}

	if (save) {
		*save = parent;
	}

	return NULL;
}

static rbtree_node_t *rb_search(uintptr_t key, rbtree_node_t *root)
{
	return rb_search_auxiliary(key, root, NULL);
}

static rbtree_node_t *rb_insert(uintptr_t key, void* data, rbtree_node_t *root)
{
	rbtree_node_t *parent = NULL;
	rbtree_node_t *node = NULL;

	parent = NULL;

	if ((node = rb_search_auxiliary(key, root, &parent))) {
		return root;
	}

	node = rb_new_node(key, data);
	node->parent = parent;
	node->left = node->right = NULL;
	node->color = RED;

	if(parent) {
		if (parent->key > key) {
			parent->left = node;
		}else {
			parent->right = node;
		}
	}else {
		root = node;
	}
	return rb_insert_rebalance(node, root);
}

static rbtree_node_t *rb_insert_rebalance(rbtree_node_t *node, rbtree_node_t *root)
{
	rbtree_node_t *parent;
	rbtree_node_t *gparent;
	rbtree_node_t *uncle;
	rbtree_node_t *tmp;
	
	while ((parent = node->parent) && parent->color == RED) {
		gparent = parent->parent;
		
		if (parent == gparent->left) {
			uncle = gparent->right;

			if (uncle && uncle->color == RED) {
				uncle->color = BLACK;
				parent->color = BLACK;
				gparent->color = RED;

				node = gparent;
			}else {
				if (parent->right == node) {
					root = rb_rotate_left(parent, root);
					tmp = parent;
					parent = node;
					node = tmp;
				}
				parent->color = BLACK;
				gparent->color = RED;
				root = rb_rotate_right(gparent, root);
			}
		}else {
			uncle = gparent->left;
			if (uncle && uncle->color == RED) {
				uncle->color = BLACK;	
				parent->color = BLACK;
				gparent->color = RED;
				node = gparent;
			}else {
				if (parent->left == node) {
					root = rb_rotate_right(parent, root);
					tmp = parent;
					parent = node;
					node = tmp;
				}
				parent->color = BLACK;
				gparent->color = RED;
				root = rb_rotate_left(gparent, root);
			}
		}
	}
	root->color = BLACK;
	return root;
}
static rbtree_node_t *rb_erase(uintptr_t key, rbtree_node_t *root)
{
	rbtree_node_t *child;	
	rbtree_node_t *parent;
	rbtree_node_t *old; 
	rbtree_node_t *left;
	rbtree_node_t *node;

	color_t color;

	if (!(node = rb_search_auxiliary(key, root, NULL))) {
		return root;
	}		

	old = node; 
	
	if (node->left && node->right) { 
		node = node->right;
		while ((left = node->left) != NULL) {
			node = left;
		}
		child = node->right;
		parent = node->parent;
		color = node->color; 

		if (child) {
			child->parent = parent;
		}

		if (parent) {
			if (parent->left == node) {
				parent->left = child;
			}else {
				parent->right = child;
			}
		}else {
			root = child;
		}

		if (node->parent == old) {
			parent = node;
		}

		node->parent = old->parent;
		node->color = old->color;
		node->right = old->right;
		node->left = old->left;

		if (old->parent) {
			if (old->parent->left == old) {
				old->parent->left = node;
			}else {
				old->parent->right = node;
			}
		}else {
			root = node;
		}
		
		old->left->parent = node;

		if (old->right) {
			old->right->parent = node;
		}
	}else {
		if (!node->left) {
			child = node->right;
		}else if (!node->right) {
			child = node->left;
		}

		parent = node->parent;
		color = node->color;

		if (child) {
			child->parent = parent;
		}
	
		if (parent) {
			if (parent->left == node) {
				parent->left = child;
			}else {
				parent->right = child;
			}
		}else {
			root = child;
		}
	}
	free(old);
	
	if (color == BLACK) {
		root = rb_erase_rebalance(child, parent, root);
	}

	return root;
}
static rbtree_node_t *rb_erase_rebalance(rbtree_node_t *node, rbtree_node_t *parent, rbtree_node_t *root)
{
	rbtree_node_t *other;
	rbtree_node_t *o_left;
	rbtree_node_t *o_right;
	while ((!node || node->color == BLACK) && node != root) {
		if (parent->left == node) {
			other = parent->right;
			if (other->color == RED) {
				other->color = BLACK;
				parent->color = RED;
				root = rb_rotate_left(parent, root);
				other = parent->right;
			}

			if ((!other->left || other->left->color == BLACK) &&
						(!other->right || other->right->color == BLACK)) {
				other->color = RED;
				node = parent;
				parent = node->parent;  
			}else {
				if (!other->right ||  other->right->color == BLACK) {
					if ((o_left = other->left)) {
						o_left->color = BLACK;
					}

					other->color = RED;
					root = rb_rotate_right(other, root);
					other = parent->right;
				}
				other->color = parent->color;
				parent->color = BLACK;
				if (other->right) {
					other->right->color = BLACK;
				}

				root = rb_rotate_left(parent, root);
				node = root;
				break;
			}
		}else {
			other = parent->left;
			if (other->color == RED) {
				other->color = BLACK;
				parent->color = RED;
				root = rb_rotate_right(parent, root);
				other = parent->left;
			}
			
			if ((!other->left || other->left->color == BLACK) && 
					(!other->right || other->right->color == BLACK)) {
				other->color = RED;
				node = parent;
				parent = node->parent;
			}else {
				if (!other->left || other->left->color == BLACK) {
					if ((o_right = other->right)) {
						o_right->color = BLACK;
					}
					other->color = RED;
					root = rb_rotate_left(other, root);
					other = parent->left;
				}
			
				other->color = parent->color;
				parent->color = BLACK;

				if (other->left) {
					other->left->color = BLACK;
				}

				root = rb_rotate_right(parent, root);
				node = root;
				break;
			}
		}
	}
	
	if (node) {
		node->color = BLACK;
	}
	return root;
}


static int rbtree_empty(rbtree_node_t *root)
{
	return root == NULL;
}
//找到最小节点，就在根节点最左边的左子树
static rbtree_node_t *rb_min(struct rbtree_node_st *root)
{

	if (NULL == root) {
		return NULL;
	}

	rbtree_node_t *ptr = NULL;
	rbtree_node_t *current = NULL;

	ptr = root;

	while(ptr) {
		current = ptr;
		ptr = ptr->left;
	}

	return current;
}

void rbtree_init(rbtree_t *tree)
{
	tree->root = NULL;
	tree->insert = rb_insert;
	tree->min = rb_min;
	tree->search = rb_search;
	tree->erase = rb_erase;
	tree->empty = rbtree_empty;
}

