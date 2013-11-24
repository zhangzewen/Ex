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
	r->parse_state = -1;
	
	return r;	
}

http_connection_t *init_connection()
{
	http_connection_t *c;
	
	c = (http_connection_t *)malloc(sizeof(http_connection_t));
	
	if (NULL == c) {
		return NULL;
	}

	c->read = NULL;
	c->write = NULL;
	c->r = NULL;

	c->fd = -1;
	
	return c;
}
