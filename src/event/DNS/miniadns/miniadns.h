#ifndef DNS_HEADER_INCLUDED
#define DNS_HEADER_INCLUDED

enum dns_query_type {DNS_A_RECORD = 0x01, DNS_MX_RECORD = 0x0f};


#define	DNS_QUERY_TIMEOUT	30	/* Query timeout, seconds */

/*
 * The API
 */
struct dns;

typedef void (*dns_callback_t)(void *context, const char *name, const unsigned char *addr, size_t addrlen);

extern struct dns *dns_init(dns_callback_t callback);

extern void	dns_poll(void *context, struct dns *dns);

int dns_get_fd(struct dns *dns);

void dns_request(struct dns *dns, const char *url, const char *name, enum dns_query_type qtype);

#endif 
