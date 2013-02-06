#include "http_error.h"
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
int error_sys(const char *text)
{
	fprintf(stderr,"%s at %s:%d desc:%s\n",strerror(errno), __FILE__, __LINE__, text);	
	return -1;
}
void error_quit(const char *text)
{
	fprintf(stderr,"%s at %s:%d desc:%s\n, and the program will be exit!\n",strerror(errno), __FILE__, __LINE__, text);	
	exit(-1);
}
