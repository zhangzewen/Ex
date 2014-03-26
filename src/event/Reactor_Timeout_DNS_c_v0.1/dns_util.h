#ifndef _HTTP_DNS_UTIL_H_INCLUDED_
#define _HTTP_DNS_UTIL_H_INCLUDED_

#if 0
#define T_A 1 //Ipv4 address
#define T_NS 2 //Nameserver
#define T_CNAME 5 // canonical name
#define T_SOA 6 /* start of authority zone */
#define T_PTR 12 /* domain name pointer */
#define T_MX 15 //Mail server


#include "http_resolver.h"
//DNS header structure
struct dns_header 
{
	unsigned short id; // identification number

	unsigned char rd :1; // recursion desired
	unsigned char tc :1; // truncated message
	unsigned char aa :1; // authoritive answer
	unsigned char opcode :4; // purpose of message
	unsigned char qr :1; // query/response flag

	unsigned char rcode :4; // response code
	unsigned char cd :1; // checking disabled
	unsigned char ad :1; // authenticated data
	unsigned char z :1; // its z! reserved
	unsigned char ra :1; // recursion available

	unsigned short q_count; // number of question entries
	unsigned short ans_count; // number of answer entries
	unsigned short auth_count; // number of authority entries
	unsigned short add_count; // number of resource entries
};

//Constant sized fields of query structure
struct question 
{
	unsigned short qtype;
	unsigned short qclass;
};

//Constant sized fields of the resource record structure
#pragma pack(push, 1)
struct r_data 
{
	unsigned short type;
	unsigned short _class;
	unsigned int ttl;
	unsigned short data_len;
};
#pragma pack(pop)

//Pointers to resource record contents
struct res_record
{
	unsigned char *name;
	struct r_data *resource;
	unsigned char *rdata;
};

//Structure of a Query
typedef struct
{
	unsigned char *name;
	struct question *ques;
} query;

unsigned char *ReadName(unsigned char *reader, unsigned char *buffer, int *count);
void create_dns_query(unsigned char *host, int query_type, unsigned char *buf, int *question_len);
void ChangetoDnsNameFormat(unsigned char* dns,  unsigned char* host);
void ChangeDnsNameFormatoString(unsigned char *dns, unsigned char *host);
void parse_dns(int fd, short events, void *arg);
unsigned char *ReadName(unsigned char *reader, unsigned char *buffer, int *count);

#endif

// Structs and defines common to most of DNS parse.

#ifndef __DP_MAIN
#define __DP_MAIN
#endif

#include <pcap.h>
// For standard int type declarations.
#include <stdint.h>

// Verbosity flags. Switch which function is defined to add or remove
// various output printfs from the source. These are all for debugging
// purposes.
//#define VERBOSE(A) A
#define VERBOSE(A)
//#define DBG(A) A fflush(stdout);
#define DBG(A)
//#define SHOW_RAW(A) A
#define SHOW_RAW(A) 
// There are a lot of DBG statements in the tcp and ip_fragment sections.
// When debugging those areas, it's really nice to know what's going on
// exactly at each point.

// Get the value of the BITth bit from byte offset O bytes from base B.
#define GET_BIT(B,O,BIT) (uint8_t)(((*(B+O)) & (1 << (BIT))) >> BIT )
// Get a two byte little endian u_int at base B and offset O.
#define LE_U_SHORT(B,O) (uint16_t)((B[O]<<8)+B[O+1])
// Get a four byte little endian u_int at base B and offset O.
#define LE_U_INT(B,O) (uint32_t)((B[O]<<24)+(B[O+1]<<16)+(B[O+2]<<8)+B[O+3])
// Get the DNS tcp length prepended field.
#define TCP_DNS_LEN(P,O) ((P[O]<<8) + P[O+1])

// Pre-declarations.
struct tcp_info;
struct ip_fragment;

#define MAX_EXCLUDES 100
// Globals passed in via the command line.
// I don't really want these to be globals, but libpcap doesn't really 
// have the mechanism I need to pass them to the handler.
typedef struct {
    uint16_t EXCLUDED[MAX_EXCLUDES];
    uint16_t EXCLUDES;
    char SEP;
    char * RECORD_SEP;
    int AD_ENABLED;
    int NS_ENABLED;
    int COUNTS;
    int PRETTY_DATE;
    int PRINT_RR_NAME;
    int MISSING_TYPE_WARNINGS;
    char * TCP_STATE_PATH;
    uint32_t DEDUPS;
    struct tcp_info * tcp_sessions_head;
    struct ip_fragment * ip_fragment_head;
    unsigned long long * dedup_hashes;
    uint32_t dedup_pos;
    
} config;

// Holds the information for a dns question.
typedef struct dns_question {
    char * name;
    uint16_t type;
    uint16_t cls;
    struct dns_question * next;
} dns_question;

// Holds the information for a dns resource record.
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

// Holds general DNS information.
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

// Including these earlier leads to all sorts of circular dependencies.
#include "tcp.h"
#include "network.h"

#define FORCE 1

// Parse DNS from from the given 'packet' byte array starting at offset 'pos', 
// with libpcap header information in 'header'. 
// The parsed information is put in the 'dns' struct, and the 
// new pos in the packet is returned. (0 on error).
// The config struct gives needed configuration options.
// force - Force fully parsing the dns data, even if 
//   configuration parameters mean it isn't necessary. If this is false,
//   the returned position may not correspond with the end of the DNS data. 
uint32_t dns_parse(uint32_t pos, struct pcap_pkthdr *header, 
                   uint8_t *packet, dns_info * dns,
                   config * conf, uint8_t force);
// Print the information in the given packet information objects according
// to the settings in the configuration struct.
void print_summary(ip_info * ip, transport_info * trns, dns_info * dns,
                   struct pcap_pkthdr * header, config * conf);
// Print packet bytes in hex.
// max_len - Maximum packet offset.
// packet - pointer to the packet data.
// start - start offset
// end - end offset (if farther than max_len, printing stops at max_len).
// wrap - How many bytes to print per line.
void print_packet(uint32_t max_len, uint8_t *packet,
                  uint32_t start, uint32_t end, u_int wrap);
#endif
