
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <string.h>
#include "http_parse.h"
#include "Define_Macro.h"

#ifndef CREATE_HTTP_CONTENT_
#define CREATE_HTTP_CONTENT(src,attr) strncat(src,attr,strlen(attr))														

char *create_http_request(http_request request)
{
	char * request_content = NULL;
	request_content = (char *)malloc(sizeof(char)*4096);
	if(NULL == request_content)
	{
		return NULL;
	}
	CREATE_HTTP_CONTENT(request_content,request->method);						
	CREATE_HTTP_CONTENT(request_content," ");												//GET / HTTP/1.1\r\n	
	CREATE_HTTP_CONTENT(request_content,request->url);							//Host: www.baidu.com\r\n
	CREATE_HTTP_CONTENT(request_content," ");												//
	CREATE_HTTP_CONTENT(request_content,request->version);						
	CREATE_HTTP_CONTENT(request_content,"\r\n");
	CREATE_HTTP_CONTENT(request_content,"Host: ");
	CREATE_HTTP_CONTENT(request_content,request->host);
	CREATE_HTTP_CONTENT(request_content,"\r\n\r\n");

	return request_content;
}

char *create_http_response(http_response response)
{
	char * response_content = NULL;
	response_content = (char *)malloc(sizeof(char)*4096);
	if(NULL == response_content)
	{
		return NULL;
	}
	CREATE_HTTP_CONTENT(response_content,response->version);						
	CREATE_HTTP_CONTENT(response_content," ");												//GET / HTTP/1.1\r\n	
	CREATE_HTTP_CONTENT(response_content,response->status_code);							//Host: www.baidu.com\r\n
	CREATE_HTTP_CONTENT(response_content,"\r\n");												//
	CREATE_HTTP_CONTENT(response_content, "Date: ");
	CREATE_HTTP_CONTENT(response_content, response->date);
	CREATE_HTTP_CONTENT(response_content,"\r\n");
	CREATE_HTTP_CONTENT(response_content, "Server: ");
	CREATE_HTTP_CONTENT(response_content, response->server);
	CREATE_HTTP_CONTENT(response_content,"\r\n");
	CREATE_HTTP_CONTENT(response_content, "Content-Length: ");
	CREATE_HTTP_CONTENT(response_content, response->content_length);
	CREATE_HTTP_CONTENT(response_content,"\r\n\r\n");
	return response_content;
}

#undef CREATE_HTTP_CONTENT
#if 0
int parse(const char *src,const char *tag1,const char *tag2,char **position)
{
	char *ptr_tag1 = NULL;
	char *ptr_tag2 = NULL;
	if(NULL == src) {
		return 1;
	}
	if ((ptr_tag1 = strstr(src,tag1)) == NULL)
	{
		return -1;
	}
	
	if((ptr_tag2 = strstr(ptr_tag1,tag2)) == NULL)
	{
		return -2;
	}
	*position = ptr_tag1 - strlen(tag1);
	return ptr_tag2-ptr_tag1;
}
#endif

/*
* just for create struct http_request_s and struct http_respose_s 
*/
#define CREATE_HTTP_STRUCT(ptr, disc, attr, src)  do{ \
	if(strncmp(ptr, disc, strlen(disc)) == 0){  \
		strcpy(ptr->attr, src + strlen(disc));\
	}\
}while(0)

int parse_http_request_core(const char *src, http_request request)
{
	
	CREATE_HTTP_STRUCT(request, "Accept: ", accept, src);
	CREATE_HTTP_STRUCT(request, "Accept-Language: ", accept_language, src);
	CREATE_HTTP_STRUCT(request, "Accept-Encoding: ", accept_encoding, src);
	CREATE_HTTP_STRUCT(request, "Connection: ", connection, src);
	CREATE_HTTP_STRUCT(request, "Host: ", host, src);
	CREATE_HTTP_STRUCT(request, "User-Agent: ", user_agent, src);
	return 0;
}


int parse_http_response_core(const char *src, http_response response)
{
	
	CREATE_HTTP_STRUCT(request, "Server: ", server, src);
	CREATE_HTTP_STRUCT(request, "Date: ", date, src);
	CREATE_HTTP_STRUCT(request, "Content-Length: ", content_length, src);
	return 0;
}

int parse_http_request(const char *request, http_request request)
{
	
	char buff[4086] = {0};
	char *token;
	strncpy(buff, request, strlen(request));
	token = strtok(buff, "\r\n");
	sscanf(token, "%s %s %s", request->method, request->url, request->version);
	token = strtok(NULL, "\r\n");
	while(token != NULL) {
		puts(token);
		parse_http_request_core(token, request)
		token = strtok(NULL, "\r\n");
	}
}


int parse_http_response(const char *response, http_response response)
{
	
	char buff[4086] = {0};
	char *token;
	strncpy(buff, request, strlen(response));
	token = strtok(buff, "\r\n");
	sscanf(token, "%s %s %s", response->version, request->status_code, request->status_code_desc);
	token = strtok(NULL, "\r\n");
	while(token != NULL) {
		puts(token);
		parse_http_response_core(token, request)
		token = strtok(NULL, "\r\n");
	}
}


#if 0
int extract_content_length(char *buffer, int size)
{
	char *clen = strstr(buffer, CONTENT_LENGTH);
	char *content_buffer = NULL;
	char *buf_len;
	int inc = 0;
	int i = 0;

	/* Allocate the room */
	buf_len = (char *) malloc(40);

	/* Pattern not found */
	if (!clen)
		return 0;

	/* Content-Length extraction */
	while (*(clen++) != ':') ;
	content_buffer = clen;
	while (*(clen++) != '\r' && *clen != '\n')
		inc++;
	for (i = 0; i < inc; i++)
		strncat(buf_len, content_buffer + i, 1);
	i = atoi(buf_len);
	free(buf_len);
	return i;
}


int extract_status_code(char *buffer, int size)
{
	char *buf_code;
	char *begin;
	char *end = buffer + size;
	int inc = 0;

	/* Allocate the room */
	buf_code = (char *) malloc(10);

	/* Status-Code extraction */
	while (buffer < end && *buffer++ != ' ') ;
	begin = buffer;
	while (buffer < end && *buffer++ != ' ')
		inc++;
	strncat(buf_code, begin, inc);
	inc = atoi(buf_code);
	free(buf_code);
	return inc;
}

char *extract_html(char *buffer, int size_buffer)
{
	char *end = buffer + size_buffer;
	char *cur;

	for (cur = buffer; cur + 3 < end; cur++)
	{
		if (*cur == '\r' && *(cur + 1) == '\n' && *(cur + 2) == '\r' && *(cur + 3) == '\n')
			return cur + 4;
	}
	return NULL;
}
#endif
#undef CREATE_HTTP_CONTEN
#endif
