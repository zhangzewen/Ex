#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<unistd.h>
#include "dns_util.h"


void create_dns_query(unsigned char *host, int query_type, unsigned char *buf)
{
	
	struct dns_header *dns;
	unsigned char *qname = NULL;
	struct question* qinfo = NULL;
	memset(buf, '\0', 65536);
	dns = (struct dns_header*)buf;
	dns->id = (unsigned short) htons(getpid());
	dns->qr = 0;
	dns->opcode = 0;
	dns->aa = 0;
	dns->tc = 0;
	dns->rd = 1;
	dns->ra = 0;
	dns->z = 0;
	dns->ad = 0;
	dns->cd = 0;
	dns->rcode = 0;
	dns->q_count = htons(1);
	dns->ans_count = 0;
	dns->auth_count = 0;
	dns->add_count = 0;

	qname = (unsigned char *)&buf[sizeof(struct dns_header)];

	ChangetoDnsNameFormat(qname, host);

	qinfo = (struct question *)&buf[sizeof(struct dns_header) + (strlen((const char *)qname) + 1)];	
	
	qinfo->qtype = htons(query_type);
	qinfo->qclass = htons(1);
}

void parse_response(unsigned char *buf)
{
}



#if 0
u_char* ReadName(unsigned char* reader,unsigned char* buffer,int* count)
{
}
#endif

#if 0
//
//这个先不做，先默认是8.8.8.8:53
//

/*
 * Get the DNS servers from /etc/resolv.conf file on Linux
 * 在初始化的时候，从/etc/resolv.conf读取dns server ，设置标志位，使用gethostbyname（如果是url的话），成功，标志位为1，并把该dnsserver添加到http_resolve_t中
 * 对于端口，如果有端口就使用端口，没有就默认给53
 * */
void get_dns_servers(struct dns_server** DSserver)
{
	struct dns_server *new;
	FILE *fp;
	char line[200] , *p;
	if((fp = fopen("/etc/resolv.conf" , "r")) == NULL)
	{
		printf("Failed opening /etc/resolv.conf file \n");
	}

	while(fgets(line , 200 , fp))
	{
		if(line[0] == '#')
		{
			continue;
		}
		if(strncmp(line , "nameserver" , 10) == 0)
		{
			p = strtok(line , " ");
			p = strtok(NULL , " ");
			new = (struct dns_server *)malloc(sizeof(struct dns_server));
			if (NULL == new) {
				continue;
			}
			strcpy(DServer->host, p);
			strcpy(DServer->dot_addr, p);
			DServer->port = 53;
			DServer->quick = 1;
		}
	}
}
#endif

/*
 * This will convert "www.google.com" to "\3www\6google\3com\0" 
 * got it :)
 * */
void ChangetoDnsNameFormat(unsigned char* dns,unsigned char* host) 
{
	int lock = 0 , i;
	strcat((char*)host,".");

	for(i = 0 ; i < strlen((char*)host) ; i++) 
	{
		if(host[i]=='.') 
		{
			*dns++ = (unsigned char)(i-lock);
			for(;lock<i;lock++) 
			{
				*dns++=host[lock];
			}
			lock++; //or lock=i+1;
		}
	}
	*dns++='\0';
}

/*
*/
void ChangeDnsNameFormatoString(unsigned char *dns, unsigned char *host)
{
	int i = 0;
	int j = 0;
	char p = '\0';
	
	for(i=0;i<(int)strlen((const char*)dns);i++) 
	{
		p=dns[i];
		for(j=0;j<(int)p;j++) 
		{
			dns[i]=dns[i+1];
			i=i+1;
		}
		dns[i]='.';
	}
	dns[i-1]='\0'; //remove the last dot
}
