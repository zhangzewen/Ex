#ifndef _HTTP_VECTOR_H_INCLUDED__
#define _HTTP_VECTOR_H_INCLUDED__


typedef struct vector_st{
	unsigned int total;
	unsigned int current;
	void **data;
}vector;


#define VECTOR_DEFAULT_TOTAL 20
#define VECTOR_INCREASE_SIZE 40


vector* vector_create();

void vector_free(vector *v);

void *vector_push(vector *v);

int vector_empty(vector *v);
void *vector_get(vector *v, unsigned int index);


#endif
