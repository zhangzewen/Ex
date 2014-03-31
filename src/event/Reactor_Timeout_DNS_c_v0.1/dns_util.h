#ifndef _HTTP_DNS_UTIL_H_INCLUDED__
#define _HTTP_DNS_UTIL_H_INCLUDED__

#define T_A 1
#define T_NS 2
#define T_CNAME 3
#define T_AAAA 4
#define T_SOA 6
#define T_PTR 12
#define T_MX 15


#if 1
typedef char *rr_data_parser(const uint8_t*, uint32_t, uint32_t, uint16_t, uint32_t);

typedef struct{
	uint16_t cls;
	uint16_t rtype;
	rr_data_parser *parser;
	const char *name;
	const char *doc;
	unsigned long long count;
}rr_parser_container;

rr_parser_container *find_parse(uint16_t, uint16_t);

char *read_dns_name(uint8_t *, uint32_t, uint32_t);

rr_data_parser opts;
rr_data_parser escape;

extern rr_parser_container rr_parsers[];
rr_parser_container default_rr_parse;

void print_parsers();
void print_parse_usage();
#endif


typedef struct dns_question {
    char * name;
    uint16_t type;
    uint16_t cls;
    struct dns_question * next;
} dns_question;

typedef struct dns_rr {
    char * name;
    uint16_t type;
    uint16_t cls;
    const char * rr_name;
    uint16_t ttl;
    uint16_t rdlength;
    uint16_t data_len;
    char * data;
    struct dns_rr * next;
} dns_rr;

typedef struct {
    uint16_t id;
    char qr;
    char AA;
    char TC;
    uint8_t rcode;
    uint8_t opcode;
    uint16_t qdcount;
    dns_question * queries;
    uint16_t ancount;
    dns_rr * answers;
    uint16_t nscount;
    dns_rr * name_servers;
    uint16_t arcount;
    dns_rr * additional;
} dns_info;


struct dns_header {
	unsigned short id;
	
	unsigned char rd :1;
	unsigned char tc :1;
	unsigned char aa :1;
	unsigned char opcode :4;
	unsigned char qr :1;
	
	unsigned char rcode :4;
	unsigned char z :3;
	unsigned char ra :1;

	unsigned short q_count;
	unsigned short ans_count;
	unsigned short auth_count;
	unsigned add_count;
};

struct question
{
	unsigned short qtype;
	unsigned short qclass;
};



uint32_t dns_parse(uint32_t pos, uint8_t *packet, dns_info * dns, uint32_t len/*dns packet len*/);

#endif
