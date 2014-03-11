#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<unistd.h>
#include "dns_util.h"
#include "http_resolver.h"


unsigned char *ReadName(unsigned char *reader, unsigned char *buffer, int *count);
void ChangetoDnsNameFormat(unsigned char* dns, const unsigned char* host);
void ChangeDnsNameFormatoString(unsigned char *dns, unsigned char *host);
void create_dns_query(const unsigned char *host, int query_type, unsigned char *buf, int *question_len)
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
	*question_len = strlen((const char *)qname);
	return;
}
#if 0
void parse_response(unsigned char *buf, size_t qname_len) //strlen((const char *)qname) == qname_len ,and qname end with '\0', so it will add 1 when count!
{
	struct dns_header *dns = NULL;
	
	nread = read(
	
	dns = (struct dns_header *)buf;


	reader = &buf[sizeof(struct dns_header) + (qname_len + 1) + sizeof(struct question)]; 

#if 0
	fprintf(stderr, "The response contains: ");
	fprintf(stderr, "\n %d Questions.", noths(dns->q_count));
	fprintf(stderr, "\n %d Answers", ntohs(dns->q_count));
	fprintf(stderr, "\n %d Authoritative servers.", ntohs(dns->auth_count));
	fprintf(stderr, "\n %d Additional records\n\n", ntohs(dns->add_count));
#endif
}
#endif


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
void ChangetoDnsNameFormat(unsigned char* dns, const unsigned char* host) 
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

#if 0
void parse_dns(const unsigned char *buf, size_t question_len)
#endif

void parse_dns(int fd, short events, void *arg)
{
	struct resolver_result* result = (struct resolver_result *)arg;
	struct dns_header *dns = NULL;
	struct res_record answers[20];
	struct res_record auth[20];
	struct res_record addit[20];
	struct sockaddr_in a;
	int i = 0;
	int j = 0;
		
	
	unsigned char buf[65536] = {0};
	ssize_t nread = 0;

	//read data
	nread = read(fd, buf, 65536);
	
	if (nread < 0) {
		fprintf(stderr, "Wrong Dns response!\n");
		return;
	}
	
	dns = (struct dns_header *)buf;
	
	unsigned char *reader = NULL;

	reader = &buf[sizeof(struct dns_header) + (result->question_len + 1) + sizeof(struct question)];

	printf("\n The respose contains:");
	printf("\n Questions: %d", ntohs(dns->q_count));
	printf("\n Answers: %d", ntohs(dns->ans_count));
	printf("\n Authoritative Servers: %d", ntohs(dns->auth_count));
	printf("\n Additional records: %d", ntohs(dns->add_count));

	int stop = 0;

	for (i = 0; i< ntohs(dns->ans_count); i++)
	{
		answers[i].name = ReadName(reader, buf, &stop);
		reader = reader + stop;
		
		answers[i].resource = (struct r_data *)(reader);
		reader = reader + sizeof(struct r_data);

		if (ntohs(answers[i].resource->type) == 1) {
			answers[i].rdata = (unsigned char *)malloc(ntohs(answers[i].resource->data_len));
			
			for(j = 0; j < ntohs(answers[i].resource->data_len); j++)
			{
				answers[i].rdata[j] = reader[j];
			}

			answers[i].rdata[ntohs(answers[i].resource->data_len)] = '\0';
			reader = reader + ntohs(answers[i].resource->data_len);
		}

		else
		{
			addit[i].rdata = ReadName(reader, buf, &stop);
			reader += stop;
		}
	}


	printf("\n Answer Records: %d\n", ntohs(dns->ans_count));

	for (i = 0; i < ntohs(dns->ans_count); i++)
	{
		printf("Name: %s", answers[i].name);

		if (ntohs(answers[i].resource->type) == T_A) {
			long *p;
			p = (long *)answers[i].rdata;

			a.sin_addr.s_addr = (*p);
			printf("has IPv4 address : %s", inet_ntoa(a.sin_addr));
		}

		if (ntohs(answers[i].resource->type) == 5) {
			printf("has alias name : %s", answers[i].rdata);
		}

	}

	printf("\nAuthoritive Records: %d\n", ntohs(dns->auth_count));
	for (i = 0; i < ntohs(dns->auth_count); i++)
	{
		printf("Name: %s", auth[i].name);

		if (ntohs(auth[i].resource->type) == 2)
		{
			printf("has nameserver : %s", auth[i].rdata);
		}
		
		printf("\n");
	}


	printf("\nAdditional Records: %d\n", ntohs(dns->add_count));

	for (i = 0; i < ntohs(dns->add_count); i++) 
	{
		printf("Name: %s", addit[i].name);
	
		if (ntohs(addit[i].resource->type) == 1) {
			long *p;
			p = (long *)addit[i].rdata;
			a.sin_addr.s_addr = (*p);
			printf("has IPv4 address: %s", inet_ntoa(a.sin_addr));
		}
		printf("\n");
	}
	close(fd);
}


unsigned char *ReadName(unsigned char *reader, unsigned char *buffer, int *count)
{
	unsigned char *name;
	unsigned int p = 0;
	unsigned int jumpd = 0;
	unsigned int offset = 0;
	int i = 0;
	int j = 0;

	*count = 1;

	name = (unsigned char *)malloc(256);

	name[0] = '\0';

	while(*reader != 0)
	{
		if (*reader >= 192) {
			offset = (*reader) * 256 + *(reader + 1) - 49152;
			reader = buffer + offset -1;

			jumpd = 1;
		}	else {
			name[p++] = *reader;
		}

		reader = reader + 1;

		if (jumpd == 0) {
			*count = *count + 1;
		}
	}

	name[p] = '\0';

	if (jumpd == 1)
	{
		*count = *count + 1;
	}

	for (i = 0; i < (int)strlen((const char *)name); i++)
	{
		p = name[i];
		for (j = 0; j < (int)p; j ++) {
			name[i] = name[i + 1];
			i = i + 1;
		}

		name[i] = '.';
	}

	name[i - 1] = '\0';
	return name;
}



