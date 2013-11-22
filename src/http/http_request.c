#include "http_request.h"
#include "Define_Macro.h"
#include <stdio.h>
#include <stdlib.h>

http_request_t* init_request()
{
	http_request_t *r = NULL;
	
	r = (http_request_t *)malloc(sizeof(http_request_t));

	if (NULL == r) {
		return NULL;
	}

	r->buffer.start = (char *)malloc(sizeof(char) * BUFFSIZE);

	if (NULL == r->buffer.start) {
		free(r);
		return NULL;
	}
	
	r->buffer.pos = r->buffer.start;	
	r->buffer.last = r->buffer.start;
	r->buffer.end = r->buffer.start + BUFFSIZE;
	
	r->c = NULL;

	r->method_start = NULL;
	r->method_end = NULL;
	
	r->path_start = NULL;	
	r->path_end = NULL;

	r->version_start = NULL;
	r->version_end = NULL;

	r->key_start = NULL;
	r->key_end = NULL;
	
	r->value_start = NULL;
	r->value_end = NULL;

	return r;	
}


