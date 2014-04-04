#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>


#include "evbuf.h"
#include "event.h"
#include "event_base.h"
#include "http_resolver.h"

int main(int argc, char *argv[])
{
	struct resolver_st* resolve = NULL;
	resolve = resolver_create();
	
	resolver_init(resolve);
	
	unsigned char name_1[60] = "www.google.com";
#if 1 
	unsigned char name_2[60] = "www.baidu.com";
	unsigned char name_3[60] = "www.csdn.com";
	unsigned char name_4[60] = "www.kuwo.com";
	unsigned char name_5[60] = "www.163.com";
	unsigned char name_6[60] = "www.qq.com";
	unsigned char name_7[60] = "ww.stackoverflow.com";
	unsigned char name_8[60] = "www.youtube.com";
	unsigned char name_9[60] = "www.openrice.com";
	unsigned char name_10[60] = "www.beecrazy.com";
	unsigned char name_11[60] = "www.zhangzewen.sinaapp.com";
#endif
	resolve_name(resolve, name_1);
#if 1
	resolve_name(resolve, name_2);
	resolve_name(resolve, name_3);
	resolve_name(resolve, name_4);
	resolve_name(resolve, name_5);
	resolve_name(resolve, name_6);
	resolve_name(resolve, name_7);
	resolve_name(resolve, name_8);
	resolve_name(resolve, name_9);
	resolve_name(resolve, name_10);
	resolve_name(resolve, name_11);
#endif
 //event_dispatch();
	event_base_loop(resolve->base, 0);

	return 0;
}
