#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "vector.h"
#include <string.h>

vector* vector_create()
{
	vector *new = NULL;
	new = calloc(1, sizeof(struct vector_st));
	
	if (NULL == new) {
			return NULL;
	}

	new->total = VECTOR_DEFAULT_TOTAL;
	new->current = 0;
	new->data = (void **)calloc(VECTOR_DEFAULT_TOTAL, sizeof(void *));
	
	if (NULL == new->data) {
		free(new);
		return NULL;
	}

	return new;
}

void vector_free(vector *v)
{
	int i = 0;
	for (i = 0; i < v->current; i++) {
		free(v->data[i]);
		v->data[i] = NULL;
	}	
	free(v->data);
}

void *vector_push(vector *v)
{
	if (v->current >= v->total) {
		vector *tmp = NULL;
		tmp = (vector *)calloc(v->total + VECTOR_INCREASE_SIZE, sizeof(void *));
		if (NULL == tmp) {
			return NULL;	
		}

		memcpy(tmp, v, (sizeof(void *) * v->current));
		tmp->total = v->current + VECTOR_INCREASE_SIZE;
		tmp->current = v->current;
		vector_empty(v);
		v = tmp;
	}	
	
	v->current++;
	return	(v->data + v->current);
}

int vector_empty(vector *v)
{
	return v->current;
}


void *vector_get(vector *v, unsigned int index)
{
	if (v == NULL || index > v->current) {
		return NULL;
	}	
	
	return v->data[index];
	
}
