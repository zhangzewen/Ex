
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
		return -1;
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
		return -1;
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
	*position = ptr_tag1;
	return ptr_tag2-ptr_tag1;
}



int Get_Version(const char *src,http_response response)
{
	char *ptr = NULL;
	char *position = NULL;
	int copy_count = 0;
	if (NULL == src)
	{
		return -1;
	}
	copy_count = parse(src,"HTTP/1.0","\r\n",&position);
	strncpy(src,position,copy_count);
	return 0;
}


int Get_date(const char *src, struct response *response)
{
	char *ptr = NULL;
	char *position = NULL;
	int copy_count = 0;
	if(NULL == src)
	{
		return -1;
	}

	copy_count = parse(src, "Date: ", "\r\n",&response);
	strncpy(src,position,copy_count);
	return 0;
}
int Get_Server();
int Get_content_length();
int Get_connection();
int Get_ETag();
int Get_Last_modified();


int Get_http_response(const char *src, struct response *response)
{	
	char buf[BUFFSIZE];
	char *token;
	strcpy(buf, src);	
	
	if((token = strtok(buf, "\r\n")) != NULL) {
		token = strtok(NULL, "\r\n");
	}
	//just get the http response head such as :HTTP/1.1 / 200 OK	
	while((token = strtok(buf, "\r\n")) != NULL) 
	{
		if(!strncasecmp(token, "", )) {
		} else if(!strncasecmp(token, "", )){
		} else if (!strncasecmp(token, "\r\n", 2)) {
			//this means that goes to the "\r\n\r\n"  and next is the respons content ,hah
			break;
		}
	}
	
	return 0;
}


char *text_position(const char * src)
{
	char *ptr;

	if(NULL == src){
		return -1;
	}

	if((ptr = strstr(src, "\r\n\r\n")) == NULL)
	{
		return -1;
	}
	return ptr;
}




int extract_content_length(char *buffer, int size)
{
	char *clen = strstr(buffer, CONTENT_LENGTH);
	char *content_buffer = NULL;
	char *buf_len;
	int inc = 0;
	int i;

	/* Allocate the room */
	buf_len = (char *) MALLOC(40);

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
	FREE(buf_len);
	return i;
}


int extract_status_code(char *buffer, int size)
{
	char *buf_code;
	char *begin;
	char *end = buffer + size;
	int inc = 0;

	/* Allocate the room */
	buf_code = (char *) MALLOC(10);

	/* Status-Code extraction */
	while (buffer < end && *buffer++ != ' ') ;
	begin = buffer;
	while (buffer < end && *buffer++ != ' ')
		inc++;
	strncat(buf_code, begin, inc);
	inc = atoi(buf_code);
	FREE(buf_code);
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
