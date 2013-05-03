#ifndef _HTTP_H__INCLUDED
#define _HTTP_H__INCLUDED
#if 0
struct http_status_code{
	unsigned int code;
	char desc[128];
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
	{405, "Method Not Allowed"},
	{406, "Not Acceptable"},
	{407, "Proxy Authentication Required"},
	{408, "Request Time-out"},
	{409, "Conflict"},
	{410, "Gone"},
	{411, "Length Required"},
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
	{501, "Not Implemented"},
	{502, "Bad Geteway"},
	{503, "Service Unavailable"},
	{504, "Gateway Time-out"},
	{505, "HTTP version not supported"},
	{506, "Variant Also Negotiates"},
	{507 ,"Insufficient Storage"},
	{516, "Not Extended"}
};
#endif
typedef struct http_request_s{
	char version[10];
	char url[1024];
	char method[64];
	char host[64];
	char accept_encoding[512];
	char accept[512];
	char accept_control[512];
	char accept_language[512];
	char connection[64];
	char user_agent[512];
}*http_request;


typedef struct http_response_s{
	char version[10];
	char status_code[5];
	char status_code_desc[128];
	char date[64];
	char server[64];
	char content_length[10];
}*http_response;


char *create_http_request(http_request request);
char *create_http_response(http_response response);
int parse_http_request_core(const char *src, http_request request);
int parse_http_response_core(const char *src, http_response response);
int parse_http_request(const char *request_str, http_request request);
int parse_http_response(const char *response_str, http_response response);
#endif
