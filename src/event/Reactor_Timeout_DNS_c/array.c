//see nginx -->ngx_array.h/c
#include <stdio.h>
#include <stdlib.h>
#include "array.h"


static int array_init(struct array_st *array, unsigned int n, size_t size)
{
	array->nelts = 0;
	array->size = size;
	array->nalloc = n;
	
	array->elts = calloc(n, size);
	
	if (NULL == array->elts) {
		return -1;
	}	

	return 0;
}

struct array_st *array_create(unsigned int n, size_t size)
{
	
	struct array_st *array;
	
	array = calloc(1, sizeof(struct array_st));
	
	if (NULL == array) {
		return NULL;
	}

	if (init_array(array, n, size) != 0) {
		free(array);
		array = NULL;
		return NULL;
	}

	return array;
	
}


void array_destroy(struct array_st *array)
{
		
}

void array_push(struct array_st *array)
{
	void *elt;
	void *new;
	
	if (array->nelts >= array->nalloc) {
			
	}
}

