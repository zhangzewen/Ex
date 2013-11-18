#include <stdio.h>
#include <stdlib.h>
#include "sds.h"


int main(int argc, char *argv[])
{
	sds ex = sdsnewlen("zhangjie", 8);
	printf("ex = (char *)%s\n", (char*)ex);
	
	ex = sdscat(ex, "www.baidu.com?name=zhangjie&password=zhangjie");
	printf("ex = (char *)%s\n", (char*)ex);
	
	return 0;
}
