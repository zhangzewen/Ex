#ifndef _HTTP_RESOLVE_H_INCLUDED__
#define _HTTP_RESOLVE_H_INCLUDED__

typedef struct {
	char host[200];
	char dot_num[128];
	int default_port;
	int port;
};


typedef struct {
	struct dns_server *DServer; //dns server
	
};






#endif
