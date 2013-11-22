#ifndef _HTTP_H__INCLUDED
#define _HTTP_H__INCLUDED
#include "http_request.h"

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


