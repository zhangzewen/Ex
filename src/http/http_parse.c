
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

#define ERROR_PAGE "<http><head></head><body><center><h1>Sorry, Page Error!</h1></center></body></http>"
void error_page(int fd, int status_code)
{
	char buff[4096] = {0};
	for (i = 0; i < 55; i++) {
		if(my_status_code[i].code == status_code) {
			break;
		}
	}
	sprintf(buff, "HTTP/1.1 %d %s\r\nServer: %s\r\nDate: %s\r\n:Content-Length: %d\r\n\r\n%s", status_code, my_status_code[i].desc, response->server, response->date, strlen(ERROR_PAGE), ERROR_PAGE);
	write(fd, buff, strlen(buff));
}


int http_process(int fd, )
{
	/*parse http_request*/
	
	/*create http_response*/
}
#endif
