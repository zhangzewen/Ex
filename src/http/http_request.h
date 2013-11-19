#ifndef _HTTP_REQUEST_H_INCLUDED_
#define _HTTP_REQUEST_H_INCLUDED_

typedef struct http_request_st http_request_t;

struct http_request_st{
	http_connection_t *c;
	
	char *url_start;
	char *url_end;
	char *method_start;
	char *method_end;
	char *version_start;
	char *version_end;

	int http_version;
};

int parse_http_request_line(http_request_t *r, http_buffer_t *buffer);

#endif


