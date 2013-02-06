#ifndef _HTTP_H__INCLUDED
#define _HTTP_H__INCLUDED
enum Method{
	GET=1,
	POST,
	HEAD,
	PUT,
	TRACE,
	OPTIONS,
	DELETE
};
struct http_status_code{
	unsigned int code;
	char desc[64];
};


struct http_status_code my_status_code[] = {
	{100, "Continue"},
	{101, "Switching Protocols"},
	{200, "OK"},
	{201, "Created"},
	{202, "Accepted"},
	{203, "Non-Authoritative Information"},
	{204, "No Content"},
	{205, "Reset Content"},
	{206, "Partial Content"},
	{207, "Multi-Status"},
	{226, "IM Used"},
	{300, "Multiple Choices"},
	{301, "Moved Permanently"},
	{302, "Found"},
	{303, "See Others"},
	{304, "Not Modified"},
	{305, "Use Proxy"},
	{306, "Unused"},
	{307, "Temporary Redirect"},
	{400, "Bad Request"},
	{401, "Unauthorized"},
	{402, "Payment Required"},
	{403, "Forbidden"},
	{404, "Not Found"},
	{405, "Method Not Allowed"},,
	{406, "Not Acceptable"},
	{407, "Proxy Authentication Required"},
	{408, "Request Time-out"},
	{409, "Conflict"},
	{410, "Gone"},
	{411, "Length Required"},,
	{412, "Precondition Failed"},
	{413, "Request Entity Too Large"},
	{414, "Request-URI Too Large"},
	{415, "Unsupported Media Type"},
	{416, "Requested range not satisfiable"},
	{417, "Expectation Failed"},
	{423, "Locked"},
	{424, "Failed Dependency"},
	{425, "Unordered Collection"},
	{426, "Upgrade Required"},
	{500, "Internal Server Error"},
	{501, "Not Implemented"},,
	{502, "Bad Geteway"},
	{503, "Service Unavailable"},
	{504, "Gateway Time-out"},
	{505, "HTTP version not supported"},
	{506, "Variant Also Negotiates"},
	{507 ,"Insufficient Storage"},
	{516, "Not Extended"}
};

typedef struct {
	char version[10];
	char url[1024];
	enum Method http_method;
	char host[64];
	unsigned int content_length;
	char http_body[0];	
}http_request;


typedef struct {
	char version[10];
	char status_code[256];
	char host[64];
	unsigned int content_length;
	char http_body[0];	
}http_response;


char *create_http_head(const char *method,const char *version,const char *url,const char *host);
int send_http_request(int sockfd,const char *http_request);
int get_http_response(int sockfd,char *data);
int parse_http(const char *test_url,char *host,char *url,char *port);
int create_http_request();
int 

#endif

#ifndef _HTML_H
#define _HTML_H

/* HTTP header tag */
#define CONTENT_LENGTH	"Content-Length:"

/* Prototypes */
extern int extract_content_length(char *buffer, int size);
extern int extract_status_code(char *buffer, int size);
extern char *extract_html(char *buffer, int size_buffer);

#endif
