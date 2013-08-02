#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "http_parse.h"

int main(int argc, char *argv[])
{
	char buff[] = "GET http://192.168.10.78:8080/books/?name=Professional%20Ajax&password=zhangjie HTTP/1.1\r\nHOST: www.baidu.com\r\nConnection: Keep-Alive\r\n\r\n";
	char *pos = buff;
	char *end = buff + strlen(buff);
	parse_http_request_line(pos, end);
	return 0;
}
