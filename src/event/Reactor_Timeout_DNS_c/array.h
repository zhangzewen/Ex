#ifndef _HTTP_ARRAY_H_INCLUDED_
#define _HTTP_ARRAY_H_INCLUDED_

//see nginx -->ngx_array.h/c


struct array_st{
	void *elts;
	unsigned int nelts;
	size_t size;
	unsigned int nalloc;
};


struct array_st *array_create(unsigned int n, size_t size);
void array_destroy(struct array_st *array);
void array_push(struct array_st *array);


#endif
