#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include "http.h"
#include <string.h>
#include "Define_Macro.h"
char *create_http_head(const char *method,const char *version,const char *url,const char *host)
{
	char tmp[4096];
	memset(tmp,0,4096);
	strncpy(tmp,method,strlen(method));
	strncat(tmp,' ',1);
	strncat(tmp,url,strlen(url));
	strncat(tmp,' ',1);
	strncat(tmp,version,strlen(version));
	strncat(tmp,"\r\n",strlen("\r\n"));
	strcat(tmp,host);
	return tmp;
}

int parse_http(const *char test_url,char *host,char *url,char *port);
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
char *create_http_head(const char *method,const char *version,const char *url,const char *host)
{
	char tmp[4096];
	memset(tmp,0,4096);
	strncpy(tmp,method,strlen(method));
	strncat(tmp,' ',1);
	strncat(tmp,url,strlen(url));
	strncat(tmp,' ',1);
	strncat(tmp,version,strlen(version));
	strncat(tmp,"\r\n",strlen("\r\n"));
	strcat(tmp,host);
	return tmp;
}

int send_http_request(int sockfd,const char *http_request);
int get_http_response(int sockfd,char *data);
struct http_response parse_http(const char *src)
{
	
}
/*src == NULL do noting 
* can not locate tag1 return -1;
* can not locate tag2 return -2
*/

int function(const char *src,const char *tag1,const char *tag2,char **position)
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



int Get_Version(const char *src,struct response * response)
{
	char *ptr = NULL;
	char *position = NULL;
	int copy_count = 0;
	if (NULL == src)
	{
		return -1;
	}
	copy_count = function(src,"HTTP/1.0","\r\n",&position);
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

	copy_count = function(src, "Date: ", "\r\n",&response);
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
	strcpy(buf, src);	
	
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


void *create_http_request(struct request *request)
{
	char * request_content = NULL;
	request_content = (char *)malloc(sizeof(char)*4096);
	if(NULL == request_content)
	{
		return -1;
	}
#ifndef CREATE_HTTP_REQUEST
#define CREATE_HTTP_REQUEST(src,attr) strncpy(src,attr,strlen(attr))														
	CREATE_HTTP_REQUEST(request_content,request->method);						
	CREATE_HTTP_REQUEST(request_content," ");												//GET / HTTP/1.1\r\n	
	CREATE_HTTP_REQUEST(request_content,request->url);							//Host: www.baidu.com\r\n
	CREATE_HTTP_REQUEST(request_content," ");												//
	CREATE_HTTP_REQUEST(request_content,request->version);						
	CREATE_HTTP_REQUEST(request_content,"\r\n");
	CREATE_HTTP_REQUEST(request_content,"Host: ");
	CREATE_HTTP_REQUEST(request_content,request->host);
	CREATE_HTTP_REQUEST(request_content,"\r\n\r\n");
	//CREATE_HTTP_REQUEST(request_content,"\r\n");

#undef CREATE_HTTP_REQUEST
	return request_content;
}

struct {
	char version[16];
	char method[16];
	char host[64];
	char url[1024];
	char connection[16];
	char accept_encoding[128];
	char accept_language[128];
	char accept[128];
}http_request;
/*
struct {
	char version[16];
	char method[16];
	char host[64];
	char url[1024];
	char other[0]; // bian chang shu zu
}http_request;

*/









struct {
	char version[16];
	char method[16];
	unsigned int status_code;
	char date[64];
	char server[64];
	char ETag[64];
	char accept_ranges[64];
	unsigned int content_length;
	char connection[16];
	char content_type[128];
}http_response;







//---------------------------------------

#include <string.h>
#include <stdlib.h>
#include "html.h"
#include "memory.h"


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
