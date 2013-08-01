#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "http_parse.h"

int main(int argc, char *argv[])
{
	char buff[] = "GET /SmartGrid/apache/htdocs/index.php HTTP/1.1\r\n";
	char *pos = buff;
	char *end = buff + strlen(buff);
	parse_http_request_line(pos, end);
	return 0;
}
