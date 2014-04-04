#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>
#include "dns_util.h"


rr_parser_container default_rr_parser = {0, 0, escape, NULL, NULL, 0}; 

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



char* escape_data(const uint8_t* packet, uint32_t start, uint32_t end)
{
	int i = 0;
	int o = 0;
	uint8_t c = 0;
	//uint8_t upper = 0;
	//uint8_t lower = 0;
	uint32_t length = 1;

	char* outstr;

	for (i = start; i < end; i++) {
		c = packet[i];

		if (c < 0x20 || c == 0x5c || c >= 0x75) {
			length += 4;
		} else {
			length += 1;
		}
	}

	outstr = (char *)malloc(sizeof(char) * length);

	if (outstr == NULL) {
		return NULL;
	}

	for (i=start; i<end; i++) {
		c = packet[i];
		if (c < 0x20 || c == 0x5c || c >= 0x7f) {
			outstr[o] = '\\';
			outstr[o+1] = 'x';
			outstr[o+2] = c/16 + 0x30;
			outstr[o+3] = c%16 + 0x30;
			if (outstr[o+2] > 0x39) outstr[o+2] += 0x27;
			if (outstr[o+3] > 0x39) outstr[o+3] += 0x27;
			o += 4;
		} else {
			outstr[o] = c;
			o++;
		}   
	}   
	outstr[o] = 0;
	return outstr;
}


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
			// 0xc0 ===> 1100 0000, 0x3f ==> 0011 1111
			if ((c & 0xc0) == 0xc0) {
				if (pos + 1 >= len) return 0;
				if (end_pos == 0) end_pos = pos + 1;
				// c & 0x3f 获取c的后六位
				pos = id_pos + ((c & 0x3f) << 8) + packet[pos+1];
				next = pos;
			} else {
				name_len++;
				pos++;
				//example for www.google.com ,that will translate 
				//to 3www6google3com0, when next == pos case 1: c && 0xc0 == 0xc0, 
				//case 2: c translate to Binary equal to 3/6/3 does
				next = next + c + 1; 
			}   
		} else {
			if (c >= '!' && c <= 'z' && c != '\\') name_len++; // i just do not know why ,but that do not effect 
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



char * mk_error(const char * msg, const uint8_t * packet, uint32_t pos,
                uint16_t rdlength) {
    char * tmp = escape_data(packet, pos, pos+rdlength);
    size_t len = strlen(tmp) + strlen(msg) + 1;
    char * buffer = malloc(sizeof(char)*len);
    sprintf(buffer, "%s%s", msg, tmp);
    free(tmp);
    return buffer;
}


#define NULL_DOC "This data is simply hex escaped. \n"\
"Non printable characters are given as a hex value(\\x30), for example."

char *escape(const uint8_t *packet, uint32_t pos, uint32_t i,
							uint16_t rdlength, uint32_t plen) {
	return escape_data(packet, pos, pos + rdlength);
}






#define A_DOC "A (IPv4 address) format\n"\
"A records are simply an IPv4 address, and are formatted as such."
char * A(const uint8_t * packet, uint32_t pos, uint32_t i,
         uint16_t rdlength, uint32_t plen) {
    char * data = (char *)malloc(sizeof(char)*16);

    if (rdlength != 4) {
        free(data);
        return mk_error("Bad A record: ", packet, pos, rdlength);
    }   
     
    sprintf(data, "%d.%d.%d.%d", packet[pos], packet[pos+1],
                                 packet[pos+2], packet[pos+3]);
		
		fprintf(stderr, "\n[%s:%d] data = %s\n", __func__, __LINE__, data);

    return data;
}

#define D_DOC "domain name like format\n"\
"A DNS like name. This format is used for many record types."
char * domain_name(const uint8_t * packet, uint32_t pos, uint32_t id_pos,
                   uint16_t rdlength, uint32_t plen) {
    char * name = read_rr_name(packet, &pos, id_pos, plen);
    if (name == NULL) 
        name = mk_error("Bad DNS name: ", packet, pos, rdlength);

		fprintf(stderr, "\n[%s:%d] name = %s\n", __func__, __LINE__, name);
    return name;
}

#define SOA_DOC "Start of Authority format\n"\
"Presented as a series of labeled SOA fields."
char * soa(const uint8_t * packet, uint32_t pos, uint32_t id_pos,
                 uint16_t rdlength, uint32_t plen) {
    char * mname;
    char * rname;
    char * buffer;
    uint32_t serial, refresh, retry, expire, minimum;
    const char * format = "mname: %s, rname: %s, serial: %d, "
                          "refresh: %d, retry: %d, expire: %d, min: %d";

    mname = read_rr_name(packet, &pos, id_pos, plen);
    if (mname == NULL) return mk_error("Bad SOA: ", packet, pos, rdlength);
    rname = read_rr_name(packet, &pos, id_pos, plen);
    if (rname == NULL) return mk_error("Bad SOA: ", packet, pos, rdlength);

    serial = (packet[pos] << 8) + packet[pos+1];
    refresh = (packet[pos+2] << 8) + packet[pos+3];
    retry = (packet[pos+4] << 8) + packet[pos+5];
    expire = (packet[pos+6] << 8) + packet[pos+7];
    minimum = (packet[pos+8] << 8) + packet[pos+9];

    // The 5 tens are for the max of ten digits for the numeric fields.
    // The format string will lose 14 chrs of format marks.
    // The +1 is for the terminating null.
    buffer = malloc(sizeof(char) * (strlen(format) + strlen(mname) +
                                    strlen(rname) + 10*5 - 14 + 1));
    sprintf(buffer, format, mname, rname, serial, refresh, retry, expire,
            minimum);
    free(mname);
    free(rname);
    return buffer;
}

#define MX_DOC "Mail Exchange record format\n"\
"A standard dns name preceded by a preference number."
char * mx(const uint8_t * packet, uint32_t pos, uint32_t id_pos,
                uint16_t rdlength, uint32_t plen) {

    uint16_t pref = (packet[pos] << 8) + packet[pos+1];
    char * name;
    char * buffer;
    uint32_t spos = pos;

    pos = pos + 2;
    name = read_rr_name(packet, &pos, id_pos, plen);
    if (name == NULL)
        return mk_error("Bad MX: ", packet, spos, rdlength);

    buffer = malloc(sizeof(char)*(5 + 1 + strlen(name) + 1));
    sprintf(buffer, "%d,%s", pref, name);
    free(name);
    return buffer;
}

#define OPTS_DOC "EDNS option record format\n"\
"These records contain a size field for warning about extra large DNS \n"\
"packets, an extended rcode, and an optional set of dynamic fields.\n"\
"The size and extended rcode are printed, but the dynamic fields are \n"\
"simply escaped. Note that the associated format function is non-standard,\n"\
"as EDNS records modify the basic resourse record protocol (there is no \n"\
"class field, for instance. RFC 2671"
char * opts(const uint8_t * packet, uint32_t pos, uint32_t id_pos,
                  uint16_t rdlength, uint32_t plen) {
    uint16_t payload_size = (packet[pos] << 8) + packet[pos+1];
    char *buffer;
    const char * base_format = "size:%d,rcode:0x%02x%02x%02x%02x,%s";
    char *rdata = escape_data(packet, pos+6, pos + 6 + rdlength);

    buffer = malloc(sizeof(char) * (strlen(base_format) - 20 + 5 + 8 +
                                    strlen(rdata) + 1));
    sprintf(buffer, base_format, payload_size, packet[2], packet[3],
                                 packet[4], packet[5], rdata);
    free(rdata);
    return buffer;
}

#define SRV_DOC "Service record format. RFC 2782\n"\
"Service records are used to identify various network services and ports.\n"\
"The format is: 'priority,weight,port target'\n"\
"The target is a somewhat standard DNS name."
char * srv(const uint8_t * packet, uint32_t pos, uint32_t id_pos,
                 uint16_t rdlength, uint32_t plen) {
    uint16_t priority = (packet[pos] << 8) + packet[pos+1];
    uint16_t weight = (packet[pos+2] << 8) + packet[pos+3];
    uint16_t port = (packet[pos+4] << 8) + packet[pos+5];
    char *target, *buffer;
    pos = pos + 6;
    // Don't read beyond the end of the rr.
    target = read_rr_name(packet, &pos, id_pos, pos+rdlength-6);
    if (target == NULL)
        return mk_error("Bad SRV", packet, pos, rdlength);

    buffer = malloc(sizeof(char) * ((3*5+1) + strlen(target)));
    sprintf(buffer, "%d,%d,%d %s", priority, weight, port, target);
    free(target);
    return buffer;
}

#define AAAA_DOC "IPv6 record format.  RFC 3596\n"\
"A standard IPv6 address. No attempt is made to abbreviate the address."
char * AAAA(const uint8_t * packet, uint32_t pos, uint32_t id_pos,
                  uint16_t rdlength, uint32_t plen) {
    char *buffer;
    uint16_t ipv6[8];
    int i;

    if (rdlength != 16) {
        return mk_error("Bad AAAA record", packet, pos, rdlength);
    }

    for (i=0; i < 8; i++)
        ipv6[i] = (packet[pos+i*2] << 8) + packet[pos+i*2+1];
    buffer = malloc(sizeof(char) * (4*8 + 7 + 1));
    sprintf(buffer, "%x:%x:%x:%x:%x:%x:%x:%x", ipv6[0], ipv6[1], ipv6[2],
                                               ipv6[3], ipv6[4], ipv6[5],
                                               ipv6[6], ipv6[7]);
		fprintf(stderr, "\n[%s:%d] buffer = %s\n", __func__, __LINE__, buffer);
    return buffer;
}



rr_parser_container rr_parsers[] = { {1, 1, A, "A", A_DOC, 0},
																		 {0, 2, domain_name, "NS", D_DOC, 0},
																		 {0, 5, domain_name, "CNAME", D_DOC, 0},
																		 {0, 6, soa, "SOA", SOA_DOC, 0},
																		 {0, 12, domain_name, "PTR", D_DOC, 0},
																		 {0, 33, srv, "SRV", SRV_DOC, 0},
																		 {1, 28, AAAA, "AAAA", AAAA_DOC, 0},
																		 {0, 15, mx, "MX", MX_DOC, 0},
																	};

rr_parser_container* find_parse(uint16_t cls, uint16_t rtype);

inline int count_parsers()
{
	return sizeof(rr_parsers) / sizeof(rr_parser_container);
}
void sort_parsers()
{
	int m = 0;
	int n = 0;
	int change = 1;
	int pcount = count_parsers();
	rr_parser_container tmp;

	for (m = 0; m < pcount - 1 && change == 1; m++) {
		change = 0;
		for (n = 0; n < pcount -1; n++) {
			if (rr_parsers[n].count < rr_parsers[n + 1].count) {
				tmp = rr_parsers[n];
				rr_parsers[n] = rr_parsers[n + 1];
				rr_parsers[n + 1] = tmp;
				change = 1;
			}
		}
	}
	
	for (m = 0; m < pcount - 1; m++) {
		rr_parsers[m].count = 0;
	}
}




unsigned int PACKETS_SEEN = 0;
#define REORDER_LIMIT 100000
rr_parser_container* find_parser(uint16_t cls, uint16_t rtype)
{
	unsigned int i = 0;
	unsigned int pcount = count_parsers();
	rr_parser_container *found = NULL;

	//Re-arrange the order of the parsers according to how often things are
	// seen every REORDER_LIMIT packets

	if (PACKETS_SEEN > REORDER_LIMIT) {
		PACKETS_SEEN = 0;
		sort_parsers();
	}

	PACKETS_SEEN++;

	while (i < pcount && found == NULL) {
		rr_parser_container pc = rr_parsers[i];

		if ((pc.rtype == rtype || pc.rtype == 0) && (pc.cls = cls || pc.cls == 0)) {
			rr_parsers[i].count++;
			found = &rr_parsers[i];
			break;
		}
		i++;
	}

	if (found == NULL) {
		found = &default_rr_parser;
	}

	found->count++;
	return found;
}




// Parse the questions section of the dns protocol.
// pos - offset to the start of the questions section.
// id_pos - offset set to the id field. Needed to decompress dns data.
// packet, header - the packet location and header data.
// count - Number of question records to expect.
// root - Pointer to where to store the question records.
uint32_t parse_questions(uint32_t pos, uint32_t id_pos, uint32_t len,
                         uint8_t *packet, uint16_t count, 
                         dns_question ** root) {
    uint32_t start_pos = pos; 
    dns_question * last = NULL;
    dns_question * current;
    uint16_t i;
    *root = NULL;
		//fprintf(stderr, "[%s:%d] pos = %d, id_pos = %d\n", __func__, __LINE__, pos, id_pos);
    for (i=0; i < count; i++) {
        current = malloc(sizeof(dns_question));
        current->next = NULL; current->name = NULL;

        current->name = read_rr_name(packet, &pos, id_pos, len);
				//fprintf(stderr, "[%s:%d] pos = %d, id_pos = %d, name = %s\n", __func__, __LINE__, pos, id_pos, current->name);
        if (current->name == NULL || (pos + 2) >= len) {
            // Handle a bad DNS name.
            fprintf(stderr, "DNS question error\n");
            char * buffer = escape_data(packet, start_pos, len);
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

				//fprintf(stderr, "question->name : %s, question->type = %d, question->cls = %d\n", current->name, current->type, current->cls);
        //VERBOSE(printf("question->name: %s\n", current->name);)
        //VERBOSE(printf("type %d, cls %d\n", current->type, current->cls);)
   }
    
    return pos;
}

// Parse an individual resource record, placing the acquired data in 'rr'.
// 'packet', 'pos', and 'id_pos' serve the same uses as in parse_rr_set.
// Return 0 on error, the new 'pos' in the packet otherwise.
uint32_t parse_rr(uint32_t pos, uint32_t id_pos, uint32_t len, 
                  uint8_t *packet, dns_rr * rr) {
    int i;
    uint32_t rr_start = pos;
    rr_parser_container * parser;
    rr_parser_container opts_cont = {0,0, opts};

    //uint32_t temp_pos; // Only used when parsing SRV records.
    //char * temp_data; // Also used only for SRV records.

    rr->name = NULL;
    rr->data = NULL;
    
    rr->name = read_rr_name(packet, &pos, id_pos, len);
		//fprintf(stderr, "[%s:%d]\n", __func__, __LINE__);
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
        rr->data = escape_data(packet, pos, len);
        return 0;
    }
    
		//fprintf(stderr, "[%s:%d]\n", __func__, __LINE__);
    if ((len - pos) < 10 ) return 0;
    
		//fprintf(stderr, "[%s:%d]\n", __func__, __LINE__);
    rr->type = (packet[pos] << 8) + packet[pos+1];
    rr->rdlength = (packet[pos+8] << 8) + packet[pos + 9];
    // Handle edns opt RR's differently.
		//fprintf(stderr, "[%s:%d]\n", __func__, __LINE__);
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

		//fprintf(stderr, "[%s:%d]\n", __func__, __LINE__);
    // Make sure the data for the record is actually there.
    // If not, escape and print the raw data.
    if (len < (rr_start + 10 + rr->rdlength)) {
		//fprintf(stderr, "[%s:%d]\n", __func__, __LINE__);
        char * buffer;
        const char * msg = "Truncated rr: ";
        rr->data = escape_data(packet, rr_start, len);
        buffer = malloc(sizeof(char) * (strlen(rr->data) + strlen(msg) + 1));
        sprintf(buffer, "%s%s", msg, rr->data);
        free(rr->data);
        rr->data = buffer;
        return 0;
    }
    // Parse the resource record data.
		//fprintf(stderr, "[%s:%d]\n", __func__, __LINE__);
    rr->data = parser->parser(packet, pos, id_pos, rr->rdlength, 
                              len);

		//fprintf(stderr, "[%s:%d]\n", __func__, __LINE__);
    return pos + rr->rdlength;
}

// Parse a set of resource records in the dns protocol in 'packet', starting
// at 'pos'. The 'id_pos' offset is necessary for putting together 
// compressed names. 'count' is the expected number of records of this type.
// 'root' is where to assign the parsed list of objects.
// Return 0 on error, the new 'pos' in the packet otherwise.
uint32_t parse_rr_set(uint32_t pos, uint32_t id_pos, uint32_t len, 
                         uint8_t *packet, uint16_t count, 
                         dns_rr ** root) {
		//fprintf(stderr, "[%s:%d] pos = %d, id_pos = %d\n", __func__, __LINE__, pos, id_pos);
    dns_rr * last = NULL;
    dns_rr * current;
    uint16_t i;
    *root = NULL; 
    for (i=0; i < count; i++) {
        // Create and clear the data in a new dns_rr object.
        current = malloc(sizeof(dns_rr));
        current->next = NULL; current->name = NULL; current->data = NULL;
        
        pos = parse_rr(pos, id_pos, len, packet, current);
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

// Parse the dns protocol in 'packet'. 
// See RFC1035
// See dns_parse.h for more info.
uint32_t dns_parse(uint32_t pos, uint8_t *buf, dns_info * dns, uint32_t len/*dns packet len*/) {
    
    //int i;
    uint32_t id_pos = pos;
   // dns_rr * last = NULL;

		if (len < 12) {
			fprintf(stderr,"Truncate Packet error! Not a complete dns packet!\n");
			return -1;
		}

    dns->id = (buf[pos] << 8) + buf[pos+1];
    dns->qr = buf[pos+2] >> 7;
    dns->AA = (buf[pos+2] & 0x04) >> 2;
    dns->TC = (buf[pos+2] & 0x02) >> 1;
    dns->rcode = buf[pos + 3] & 0x0f;
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
    dns->qdcount = (buf[pos+4] << 8) + buf[pos+5];
    dns->ancount = (buf[pos+6] << 8) + buf[pos+7];
    dns->nscount = (buf[pos+8] << 8) + buf[pos+9];
    dns->arcount = (buf[pos+10] << 8) + buf[pos+11];


		fprintf(stderr, "========================================\n");
		fprintf(stderr, "DNS query: id = %d, q_count = %d, ancount = %d, nscount = %d, arcount = %d\n",
						dns->id, dns->qdcount, dns->ancount, dns->nscount, dns->arcount);
		fprintf(stderr, "========================================\n");
		
	
		
	
    // Parse each type of records in turn.
		
		pos = parse_questions(pos+12, id_pos, len/*dns packet len*/, buf, 
				dns->qdcount, &(dns->queries));
		fprintf(stderr, "pos = %d\n", pos);
		if (pos != 0) {
			pos = parse_rr_set(pos, id_pos, len, buf, 
					dns->ancount, &(dns->answers));
		}
		if (pos != 0) {	
			pos = parse_rr_set(pos, id_pos, len, buf, 
					dns->nscount, &(dns->name_servers));
		}

		if (pos != 0) {
			pos = parse_rr_set(pos, id_pos, len, buf, 
					dns->arcount, &(dns->additional));
		}
		return pos;
}


//unsigned char *ReadName(unsigned char *reader, unsigned char *buffer, int *count);
//void ChangetoDnsNameFormat(unsigned char* dns, unsigned char* host);
//void ChangeDnsNameFormatoString(unsigned char *dns, unsigned char *host);
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
}


void parse_dns(int fd, short events, void *arg)
{
	//struct resolver_result* result = (struct resolver_result *)arg;
	unsigned char buf[65536] = {0};
	dns_info dns;
	ssize_t nread = 0;

	//read data
	nread = read(fd, buf, 65536);

	//fprintf(stderr, "buff(len = %d) == %s", (int)nread, buf);
	
	if (nread < 0) {
		fprintf(stderr, "Wrong Dns response!\n");
		return ;
	}
	dns_parse(0, buf, &dns, nread);
}
