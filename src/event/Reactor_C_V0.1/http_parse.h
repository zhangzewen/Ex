#ifndef _HTTP_PARSE_H_INCLUDED_
#define _HTTP_PARSE_H_INCLUDED_
#include "http_request.h"

#if 0
void parse_method(const char *begin, const char *end, void *arg);
void parse_path(const char *begine, const char *end, void *arg);
//void parse_params(const char *path, void *arg);
void parse_version(const char *version, void *arg);
void parse_header(const char *key, const char *value, void *arg);
//void parse_formdata(const char *key, const char *value, void *arg);
//void parse_cookie(const char *key, const char *value, void *arg);
//void parse_datalength(int length, void *arg);
//void parse_data(const char *data, int length, int leftover, void *arg);
void parse_commplete(void *arg);

int parse_http_request(void *arg);
#endif
int parse_http_request_line(http_request_t *r);
#endif


