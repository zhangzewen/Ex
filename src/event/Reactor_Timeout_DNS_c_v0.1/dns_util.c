#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>
#include "dns_util.h"


void handler(uint8_t *, const uint8_t *);
void dns_rr_free(dns_rr *);
void dns_question_free(dns_question *);
uint32_t parse_rr(uint32_t, uint32_t, uint8_t *, dns_rr *);
void print_rr_section(dns_rr *, char *);
void print_packet(uint32_t, uint8_t *, uint32_t, uint32_t, u_int);
//int dedup(uint32_t, uint8_t *,_info *);

char* read_rr_name(const uint8_t* packet, uint32_t* packet_p, uint32_t id_pos, uint32_t len);


char* read_rr_name(const uint8_t* packet, uint32_t* packet_p, uint32_t id_pos, uint32_t len)
{
	uint32_t i, next, pos=*packet_p;
	uint32_t end_pos = 0;
	uint32_t name_len=0;
	uint32_t steps = 0;
	char * name;
//	int bc = 0;
//	uint8_t badchars[2000];

	// Scan through the name, one character at a time. We need to look at 
	// each character to look for values we can't print in order to allocate
	// extra space for escaping them.  'next' is the next position to look
	// for a compression jump or name end.
	// It's possible that there are endless loops in the name. Our protection
	// against this is to make sure we don't read more bytes in this process
	// than twice the length of the data.  Names that take that many steps to 
	// read in should be impossible.
	next = pos;
	while (pos < len && !(next == pos && packet[pos] == 0)
			&& steps < len*2) {
		uint8_t c = packet[pos];
		steps++;
		if (next == pos) {
			// Handle message compression.  
			// If the length byte starts with the bits 11, then the rest of
			// this byte and the next form the offset from the dns proto start
			// to the start of the remainder of the name.
			if ((c & 0xc0) == 0xc0) {
				if (pos + 1 >= len) return 0;
				if (end_pos == 0) end_pos = pos + 1;
				pos = id_pos + ((c & 0x3f) << 8) + packet[pos+1];
				next = pos;
			} else {
				name_len++;
				pos++;
				next = next + c + 1;  
			}   
		} else {
			if (c >= '!' && c <= 'z' && c != '\\') name_len++;
			else name_len += 4;
			pos++;
		}   
	}   
	if (end_pos == 0) end_pos = pos;

	// Due to the nature of DNS name compression, it's possible to get a
	// name that is infinitely long. Return an error in that case.
	// We use the len of the packet as the limit, because it shouldn't 
	// be possible for the name to be that long.
	if (steps >= 2*len || pos >= len) return NULL;

	name_len++;

	name = (char *)malloc(sizeof(char) * name_len);
	pos = *packet_p;	

	//Now actually assemble the name.
	//We've already made sure that we don't exceed the packet length, so
	// we don't need to make those checks anymore.
	// Non-printable and whitespace characters are replaced with a question
	// mark. They shouldn't be allowed under any circumstances anyway.
	// Other non-allowed characters are kept as is, as they appear sometimes
	// regardless.
	// This shouldn't interfere with IDNA (international
	// domain names), as those are ascii encoded.
	next = pos;
	i = 0;
	while (next != pos || packet[pos] != 0) {
		if (pos == next) {
			if ((packet[pos] & 0xc0) == 0xc0) {
				pos = id_pos + ((packet[pos] & 0x3f) << 8) + packet[pos+1];
				next = pos;
			} else {
				// Add a period except for the first time.
				if (i != 0) name[i++] = '.';
				next = pos + packet[pos] + 1;
				pos++;
			}
		} else {
			uint8_t c = packet[pos];
			if (c >= '!' && c <= '~' && c != '\\') {
				name[i] = packet[pos];
				i++; pos++;
			} else {
				name[i] = '\\';
				name[i+1] = 'x';
				name[i+2] = c/16 + 0x30;
				name[i+3] = c%16 + 0x30;
				if (name[i+2] > 0x39) name[i+2] += 0x27;
				if (name[i+3] > 0x39) name[i+3] += 0x27;
				i+=4;
				pos++;
			}
		}
	}
	name[i] = 0;

	*packet_p = end_pos + 1;

	return name;

}
void handler(uint8_t * args, const uint8_t *orig_packet) {
}

// Output the DNS data.
#if 0
void print_summary(dns_info * dns) {
    char date[200];
    char proto;

    uint32_t dnslength;
    dns_rr *next;
    dns_question *qnext;

    // Go through the list of queries, and print each one.
    qnext = dns->queries;
		while (qnext != NULL) {
			rr_parser_container * parser; 
			parser = find_parser(qnext->cls, qnext->type);
			if (parser->name == NULL) 
				printf("%s UNKNOWN(%s,%d)", qnext->name, parser->name, 
						qnext->type, qnext->cls);
			else 
				printf("%s %s", qnext->name, parser->name);
			qnext = qnext->next; 
		}

    // Print it resource record type in turn (for those enabled).
    print_rr_section(dns->answers, "!", conf);
    if (conf->NS_ENABLED) 
        print_rr_section(dns->name_servers, "$", conf);
    if (conf->AD_ENABLED) 
        print_rr_section(dns->additional, "+", conf);
    printf("%c%s\n", conf->SEP, conf->RECORD_SEP);
    
    dns_question_free(dns->queries);
    dns_rr_free(dns->answers);
    dns_rr_free(dns->name_servers);
    dns_rr_free(dns->additional);
    fflush(stdout); fflush(stderr);
}
#endif
// Print all resource records in the given section.
#if 0
void print_rr_section(dns_rr * next, char * name) {
    int skip;
    int i;
    while (next != NULL) {
        // Print the rr seperator and rr section name.
        skip = 0;
        // Search the excludes list to see if we should not print this
        // rtype.
        for (i=0; i < conf->EXCLUDES && skip == 0; i++) 
            if (next->type == conf->EXCLUDED[i]) skip = 1;
        if (!skip) {
            char *name, *data;
            name = (next->name == NULL) ? "*empty*" : next->name;
            data = (next->data == NULL) ? "*empty*" : next->data;
            if (conf->PRINT_RR_NAME) { 
                if (next->rr_name == NULL) 
                    // Handle bad records.
                    printf(" %s UNKNOWN(%d,%d) %s", name, next->type, 
                                                    next->cls, data);
                else
                    // Print the string rtype name with the rest of the record.
                    printf(" %s %s %s", name, next->rr_name, data);
            } else
                // The -r option case. 
                // Print the rtype and class number with the record.
                printf(" %s %d %d %s", name, next->type, next->cls, data);
        }
        next = next->next; 
    }
}

#endif

// Print packet bytes in hex.
// See dns_parse.h
#if 0
void print_packet(uint32_t max_len, uint8_t *packet,
                  uint32_t start, uint32_t end, u_int wrap) {
    int i=0;
    while (i < end - start && (i + start) < max_len) {
        printf("%02x ", packet[i+start]);
        i++;
        if ( i % wrap == 0) printf("\n");
    }
    if ( i % wrap != 0) printf("\n");
    return;
}
#endif

// Free a dns_rr struct.
void dns_rr_free(dns_rr * rr) {
    if (rr == NULL) return;
    if (rr->name != NULL) free(rr->name);
    if (rr->data != NULL) free(rr->data);
    dns_rr_free(rr->next);
    free(rr);
}

// Free a dns_question struct.
void dns_question_free(dns_question * question) {
    if (question == NULL) return;
    if (question->name != NULL) free(question->name);
    dns_question_free(question->next);
    free(question);
}

// Parse the questions section of the dns protocol.
// pos - offset to the start of the questions section.
// id_pos - offset set to the id field. Needed to decompress dns data.
// packet, header - the packet location and header data.
// count - Number of question records to expect.
// root - Pointer to where to store the question records.
uint32_t parse_questions(uint32_t pos, uint32_t id_pos, 
                         uint8_t *packet, uint16_t count, 
                         dns_question ** root) {
    uint32_t start_pos = pos; 
    dns_question * last = NULL;
    dns_question * current;
    uint16_t i;
    *root = NULL;

    for (i=0; i < count; i++) {
        current = malloc(sizeof(dns_question));
        current->next = NULL; current->name = NULL;

        current->name = read_rr_name(packet, &pos, id_pos, header->len);
        if (current->name == NULL || (pos + 2) >= header->len) {
            // Handle a bad DNS name.
            fprintf(stderr, "DNS question error\n");
            char * buffer = escape_data(packet, start_pos, header->len);
            const char * msg = "Bad DNS question: ";
            current->name = malloc(sizeof(char) * (strlen(buffer) +
                                                   strlen(msg) + 1));
            sprintf(current->name, "%s%s", msg, buffer);
            current->type = 0;
            current->cls = 0;
            if (last == NULL) *root = current;
            else last->next = current;
            return 0;
        }
        current->type = (packet[pos] << 8) + packet[pos+1];
        current->cls = (packet[pos+2] << 8) + packet[pos+3];
        
        // Add this question object to the list.
        if (last == NULL) *root = current;
        else last->next = current;
        last = current;
        pos = pos + 4;

        VERBOSE(printf("question->name: %s\n", current->name);)
        VERBOSE(printf("type %d, cls %d\n", current->type, current->cls);)
   }
    
    return pos;
}

// Parse an individual resource record, placing the acquired data in 'rr'.
// 'packet', 'pos', and 'id_pos' serve the same uses as in parse_rr_set.
// Return 0 on error, the new 'pos' in the packet otherwise.
uint32_t parse_rr(uint32_t pos, uint32_t id_pos, struct pcap_pkthdr *header, 
                  uint8_t *packet, dns_rr * rr, config * conf) {
    int i;
    uint32_t rr_start = pos;
    rr_parser_container * parser;
    rr_parser_container opts_cont = {0,0, opts};

    uint32_t temp_pos; // Only used when parsing SRV records.
    char * temp_data; // Also used only for SRV records.

    rr->name = NULL;
    rr->data = NULL;
    
    rr->name = read_rr_name(packet, &pos, id_pos, header->len);
    // Handle a bad rr name.
    // We still want to print the rest of the escaped rr data.
    if (rr->name == NULL) {
        const char * msg = "Bad rr name: ";
        rr->name = malloc(sizeof(char) * (strlen(msg) + 1));
        sprintf(rr->name, "%s", "Bad rr name");
        rr->type = 0;
        rr->rr_name = NULL;
        rr->cls = 0;
        rr->ttl = 0;
        rr->data = escape_data(packet, pos, header->len);
        return 0;
    }
    
    if ((header->len - pos) < 10 ) return 0;
    
    rr->type = (packet[pos] << 8) + packet[pos+1];
    rr->rdlength = (packet[pos+8] << 8) + packet[pos + 9];
    // Handle edns opt RR's differently.
    if (rr->type == 41) {
        rr->cls = 0;
        rr->ttl = 0;
        rr->rr_name = "OPTS";
        parser = &opts_cont;
        // We'll leave the parsing of the special EDNS opt fields to
        // our opt rdata parser.  
        pos = pos + 2;
    } else {
        // The normal case.
        rr->cls = (packet[pos+2] << 8) + packet[pos+3];
        rr->ttl = 0;
        for (i=0; i<4; i++)
            rr->ttl = (rr->ttl << 8) + packet[pos+4+i];
        // Retrieve the correct parser function.
        parser = find_parser(rr->cls, rr->type);
        rr->rr_name = parser->name;
        pos = pos + 10;
    }

    VERBOSE(printf("Applying RR parser: %s\n", parser->name);)

    if (conf->MISSING_TYPE_WARNINGS && &default_rr_parser == parser) 
        fprintf(stderr, "Missing parser for class %d, type %d\n", 
                        rr->cls, rr->type);
    
    // Make sure the data for the record is actually there.
    // If not, escape and print the raw data.
    if (header->len < (rr_start + 10 + rr->rdlength)) {
        char * buffer;
        const char * msg = "Truncated rr: ";
        rr->data = escape_data(packet, rr_start, header->len);
        buffer = malloc(sizeof(char) * (strlen(rr->data) + strlen(msg) + 1));
        sprintf(buffer, "%s%s", msg, rr->data);
        free(rr->data);
        rr->data = buffer;
        return 0;
    }
    // Parse the resource record data.
    rr->data = parser->parser(packet, pos, id_pos, rr->rdlength, 
                              header->len);
    VERBOSE(
    printf("rr->name: %s\n", rr->name);
    printf("type %d, cls %d, ttl %d, len %d\n", rr->type, rr->cls, rr->ttl,
           rr->rdlength);
    printf("rr->data %s\n", rr->data);
    )

    return pos + rr->rdlength;
}

// Parse a set of resource records in the dns protocol in 'packet', starting
// at 'pos'. The 'id_pos' offset is necessary for putting together 
// compressed names. 'count' is the expected number of records of this type.
// 'root' is where to assign the parsed list of objects.
// Return 0 on error, the new 'pos' in the packet otherwise.
uint32_t parse_rr_set(uint32_t pos, uint32_t id_pos, 
                         uint8_t *packet, uint16_t count, 
                         dns_rr ** root) {
    dns_rr * last = NULL;
    dns_rr * current;
    uint16_t i;
    *root = NULL; 
    for (i=0; i < count; i++) {
        // Create and clear the data in a new dns_rr object.
        current = malloc(sizeof(dns_rr));
        current->next = NULL; current->name = NULL; current->data = NULL;
        
        pos = parse_rr(pos, id_pos, header, packet, current, conf);
        // If a non-recoverable error occurs when parsing an rr, 
        // we can only return what we've got and give up.
        if (pos == 0) {
            if (last == NULL) *root = current;
            else last->next = current;
            return 0;
        }
        if (last == NULL) *root = current;
        else last->next = current;
        last = current;
    }
    return pos;
}

#if 0
// Generates a hash from the current packet. 
int dedup(uint32_t pos, struct pcap_pkthdr *header, uint8_t * packet,
          ip_info * ip, transport_info * trns, config * conf) {
   
    uint64_t hash = 0;
    uint64_t mask = 0xffffffffffffffff;
    uint32_t i;

    // Put the hash of the src address in the upper 32 bits,
    // and the dest in the lower 32.
    if (ip->src.vers == IPv4) {
        hash += ((uint64_t)ip->src.addr.v4.s_addr << 32);
        hash += ip->dst.addr.v4.s_addr;
    } else {
        for (i=0; i<4; i++) {
            hash += (uint64_t)ip->src.addr.v6.s6_addr32[i] << 32;
            hash += ip->dst.addr.v6.s6_addr32[i];
        }
    }
    hash += ((uint64_t)trns->srcport << 32) + trns->dstport;
    
    // Add in the payload.
    for (; pos + 8 < header->len; pos+=8) {
        hash += *(uint64_t*)(packet + pos);
    }   
    // Add in those last bytes, if any. 
    // Where doesn't matter, as long as we're consistent.
    for (; pos < header->len; pos++) {
        hash += packet[pos];
    }
    // Without this, all strings of nulls are equivalent.
    hash += header->len;
    // The hash is now done.

    if (hash == 0) {
        // Say it's not a duplicate even though it might be.
        // Since we initialize the dedup list to zero's, it's likely that
        // this will produce a false positive.
        return 0;
    }

    for (i=0; i < conf->DEDUPS; i++) {
        if (hash == conf->dedup_hashes[i]) {
            // Found a match, return the fact.
            return 1;
        }
    }
   
    // There was no match. Replace the oldest dedup.
    conf->dedup_hashes[conf->dedup_pos] = hash;
    conf->dedup_pos = (conf->dedup_pos + 1) % conf->DEDUPS;
    return 0;
}
#endif

// Parse the dns protocol in 'packet'. 
// See RFC1035
// See dns_parse.h for more info.
uint32_t dns_parse(uint32_t pos, uint8_t *packet, dns_info * dns) {
    
    int i;
    uint32_t id_pos = pos;
    dns_rr * last = NULL;

    dns->id = (packet[pos] << 8) + packet[pos+1];
    dns->qr = packet[pos+2] >> 7;
    dns->AA = (packet[pos+2] & 0x04) >> 2;
    dns->TC = (packet[pos+2] & 0x02) >> 1;
    dns->rcode = packet[pos + 3] & 0x0f;
    // rcodes > 5 indicate various protocol errors and redefine most of the 
    // remaining fields. Parsing this would hurt more than help. 
    if (dns->rcode > 5) {
        dns->qdcount = dns->ancount = dns->nscount = dns->arcount = 0;
        dns->queries = NULL;
        dns->answers = NULL;
        dns->name_servers = NULL;
        dns->additional = NULL;
        return pos + 12;
    }

    // Counts for each of the record types.
    dns->qdcount = (packet[pos+4] << 8) + packet[pos+5];
    dns->ancount = (packet[pos+6] << 8) + packet[pos+7];
    dns->nscount = (packet[pos+8] << 8) + packet[pos+9];
    dns->arcount = (packet[pos+10] << 8) + packet[pos+11];

    SHOW_RAW(
        printf("dns\n");
        print_packet(header->len, packet, pos, header->len, 2);
    )
    VERBOSE(
        printf("DNS id:%d, qr:%d, AA:%d, TC:%d, rcode:%d\n", 
               dns->id, dns->qr, dns->AA, dns->TC, dns->rcode);
        printf("DNS qdcount:%d, ancount:%d, nscount:%d, arcount:%d\n",
               dns->qdcount, dns->ancount, dns->nscount, dns->arcount);
    )

    // Parse each type of records in turn.
		pos = parse_questions(pos+12, id_pos, packet, 
				dns->qdcount, &(dns->queries));
		pos = parse_rr_set(pos, id_pos, packet, 
				dns->ancount, &(dns->answers), conf);
		pos = parse_rr_set(pos, id_pos, packet, 
				dns->nscount, &(dns->name_servers), conf);
		pos = parse_rr_set(pos, id_pos, packet, 
				dns->arcount, &(dns->additional), conf);
		return pos;
}



unsigned char *ReadName(unsigned char *reader, unsigned char *buffer, int *count);
void ChangetoDnsNameFormat(unsigned char* dns, unsigned char* host);
void ChangeDnsNameFormatoString(unsigned char *dns, unsigned char *host);
void create_dns_query(unsigned char *host, int query_type, unsigned char *buf, int *question_len)
{
	struct dns_header *dns;
	unsigned char *qname = NULL;
	struct question* qinfo = NULL;

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


void ChangetoDnsNameFormat(unsigned char* dns, unsigned char* host) 
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


static int do_parse_dns(struct resolver_result *result, unsigned char *buf)
{
#if 0
	struct dns_header *dns = NULL;
	struct res_record answers[20];
	struct res_record auth[20];
	struct res_record addit[20];
	struct sockaddr_in a;
	int i = 0;
	int j = 0;
		
	
	
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
			printf("\thas IPv4 address : %s\n", inet_ntoa(a.sin_addr));
		}

		if (ntohs(answers[i].resource->type) == 5) {
			printf("\thas alias name : %s\n", answers[i].rdata);
		}

	}

	printf("\nAuthoritive Records: %d\n", ntohs(dns->auth_count));
	for (i = 0; i < ntohs(dns->auth_count); i++)
	{
		printf("Name: %s", auth[i].name);

		if (ntohs(auth[i].resource->type) == 2)
		{
			printf("\thas nameserver : %s", auth[i].rdata);
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
			printf("\thas IPv4 address: %s", inet_ntoa(a.sin_addr));
		}
		printf("\n");
	}
	
#endif
	return 0;
}

void parse_dns(int fd, short events, void *arg)
{
#if 0
	struct resolver_result* result = (struct resolver_result *)arg;
	unsigned char buf[65536] = {0};
	ssize_t nread = 0;

	//read data
	nread = read(fd, buf, 65536);
	
	if (nread < 0) {
		fprintf(stderr, "Wrong Dns response!\n");
		return ;
	}
	
#endif
	do_parse_dns(result, buf);
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



