#include <time.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "dns.h"
#include "rbtree.h"

#define URL_LEN             256
#define	DNS_MAX			    1025	// Maximum host name 
#define	DNS_PACKET_LEN		2048	// Buffer size for DNS packet 
#define	MAX_CACHE_ENTRIES	10000	// Dont cache more than that 

struct dns 
{
	int		            sock;      // UDP socket used for queries 
	struct sockaddr_in	sa;	       // DNS server socket address 
	uint16_t	        tid;	   // Latest tid used 
	
	dns_callback_t	    callback;
	struct rb_root      active;
//	struct rb_root      cached;
};


struct query_t 
{
	struct rb_node  node;
	time_t		    expire;		        // Time when this query expire 
	uint16_t	    tid;		        // UDP DNS transaction ID, big endian 
	char		    name[URL_LEN];	    // Host name 
	unsigned char	addr[DNS_MAX];	// Host address 
	size_t		    addrlen;	        // Address length 
};


struct header 
{
	uint16_t	tid;		/* Transaction ID */
	uint16_t	flags;		/* Flags */
	uint16_t	nqueries;	/* Questions */
	uint16_t	nanswers;	/* Answers */
	uint16_t	nauth;		/* Authority PRs */
	uint16_t	nother;		/* Other PRs */
	unsigned char	data[1];	/* Data, variable length */
};



int rb_string_insert(struct rb_root *root, struct query_t *data)
{
	int    result;
	struct rb_node **new = &(root->rb_node), *parent = NULL;

	while(*new) 
	{
		struct query_t *this = container_of(*new, struct query_t, node);

		result = strcmp(data->name, this->name);

			parent = *new;
	  		if (result < 0)
	  			new = &((*new)->rb_left);
	  		else if (result > 0)
	  			new = &((*new)->rb_right);
	  		else
	  			return false;
	}

	rb_link_node(&data->node, parent, new);
	rb_insert_color(&data->node, root);

	return true;
}

struct query_t *rb_strint_search(struct rb_root *root, const char *name)
{
	int    result;
	struct rb_node *node = root->rb_node;

	while (node) 
	{
		struct query_t *data = container_of(node, struct query_t, node);
		
		result = strcmp(name, data->name);

		if (result < 0)
			node = node->rb_left;
		else if (result > 0)
			node = node->rb_right;
		else
			return data;
	}
	return NULL;
}

int rb_int_insert(struct rb_root *root, struct query_t *data)
{
	struct rb_node **new = &(root->rb_node), *parent = NULL;

	while(*new) 
	{
		struct query_t *this = container_of(*new, struct query_t, node);

		parent = *new;
		if (this->tid > data->tid)
			new = &((*new)->rb_left);
		else 
			new = &((*new)->rb_right);
	}

	rb_link_node(&data->node, parent, new);
	rb_insert_color(&data->node, root);

	return true;
}

struct query_t *rb_int_search(struct rb_root *root, uint16_t id)
{
	struct rb_node *node = root->rb_node;

	while (node) 
	{
		struct query_t *data = container_of(node, struct query_t, node);
		
	    if (data->tid > id)
			node = node->rb_left;
	    else if (data->tid < id)
			node = node->rb_right;
	    else
			return data;  // Found it 
	}
	return NULL;
}


int dns_get_fd(struct dns *dns)
{
	return dns->sock;
}

static int getdnsip(struct dns *dns)
{
	int	ret = 0;


	FILE	*fp;
	char	line[512];
	int	a, b, c, d;

	if ((fp = fopen("/etc/resolv.conf", "r")) == NULL) 
	{
		ret--;
	} 
	else 
	{
		// Try to figure out what DNS server to use 
		for (ret--; fgets(line, sizeof(line), fp) != NULL; ) 
		{
			if (sscanf(line, "nameserver %d.%d.%d.%d", &a, &b, &c, &d) == 4)
			{
				dns->sa.sin_addr.s_addr = htonl(a << 24 | b << 16 | c << 8 | d);
				ret++;
				break;
			}
		}
		fclose(fp);
	}

	return (ret);
}


struct dns *dns_init(dns_callback_t callback)
{
	struct dns	*dns;
	int     opts      = 1;
	int		rcvbufsiz = 64 * 1024;

	// FIXME resource leak here 
	if ((dns = (struct dns *) malloc(sizeof(struct dns))) == NULL)
		return (NULL);
	else if ((dns->sock = socket(PF_INET, SOCK_DGRAM, 17)) == -1)
		return (NULL);
	else if (getdnsip(dns) != 0)
		return (NULL);
	
	ioctl(dns->sock, FIONBIO, &opts);

	dns->sa.sin_family	= AF_INET;
	dns->sa.sin_port	= htons(53);
	dns->tid            = 1;
	dns->callback	    = callback;

	// Increase the receive buffer 
	setsockopt(dns->sock, SOL_SOCKET, SO_RCVBUF, (char *) &rcvbufsiz, sizeof(rcvbufsiz));

	
	memset(&dns->active, 0, sizeof(struct rb_root));
	return dns;
}

void dns_request(struct dns *dns, const char *url, const char *name, enum dns_query_type qtype)
{
	int		i, n, name_len;
	struct header	*header = NULL;
	struct query_t	*query  = NULL;

	const char 	*s;
	char pkt[DNS_PACKET_LEN], *p;
	
	query  = (struct query_t *)malloc(sizeof(struct query_t));
	if(query == NULL) return ;
	
	query->tid	  = dns->tid++;
	query->expire = time(NULL);
	
	memset(query->name, 0, URL_LEN);
	memcpy(query->name, url, URL_LEN-1);

	header		     = (struct header *) pkt;
	header->tid	     = query->tid;
	header->flags	 = htons(0x100); 
	header->nqueries = htons(1);	    
	header->nanswers = 0;
	header->nauth	 = 0;
	header->nother	 = 0;


	// Encode DNS name 
	name_len = strlen(name);
	p = (char *) &header->data;	/* For encoding host name into packet */
	
	do {
		if ((s = strchr(name, '.')) == NULL)
			s = name + name_len;

		n = s - name;			/* Chunk length */
		*p++ = n;			/* Copy length */
		for (i = 0; i < n; i++)		/* Copy chunk */
			*p++ = name[i];

		if (*s == '.')
			n++;

		name += n;
		name_len -= n;

	} while (*s != '\0');

	*p++ = 0;			/* Mark end of host name */
	*p++ = 0;			/* Well, lets put this byte as well */
	*p++ = (unsigned char) qtype;	/* Query Type */

	*p++ = 0;
	*p++ = 1;			/* Class: inet, 0x0001 */

	assert(p < pkt + sizeof(pkt));
	n = p - pkt;			/* Total packet length */
		
	if (sendto(dns->sock, pkt, n, 0, (struct sockaddr *) &dns->sa, sizeof(dns->sa)) != n) 
	{
		printf("%s error\n", name);
	}

	rb_int_insert(&dns->active, query);
}

static void fetch(const uint8_t *pkt, const uint8_t *s, int pktsiz, char *dst, int dstlen)
{
	const uint8_t	*e = pkt + pktsiz;
	int		j, i = 0, n = 0;

	
	while (*s != 0 && s < e) {
		if (n > 0)
			dst[i++] = '.';

		if (i >= dstlen)
			break;
		
		if ((n = *s++) == 0xc0) {
			s = pkt + *s;	/* New offset */
			n = 0;
		} else {
			for (j = 0; j < n && i < dstlen; j++)
				dst[i++] = *s++;
		}
	}

	dst[i] = '\0';
}

static void parse_udp(void *context, struct dns *dns, const unsigned char *pkt, int len)
{
	const unsigned char	*p, *e, *s;
	struct header   *header = NULL;
	struct query_t	*query  = NULL;
	
	uint16_t    type;
	int			found, stop, dlen, nlen;
	
	// We sent 1 query. We want to see more that 1 answer. 
	header = (struct header *) pkt;
	if (ntohs(header->nqueries) != 1)
		return;
	
	query = rb_int_search(&dns->active, header->tid);
	if(query == NULL) return ;


	/* Skip host name */
	for (e = pkt + len, nlen = 0, s = p = &header->data[0];
	    p < e && *p != '\0'; p++)
		nlen++;

#define	NTOHS(p)	(((p)[0] << 8) | (p)[1])

	/* We sent query class 1, query type 1 */
	if (&p[5] > e || NTOHS(p + 1) != DNS_A_RECORD)
		return;

	/* Go to the first answer section */
	p += 5;
	
	/* Loop through the answers, we want A type answer */
	for (found = stop = 0; !stop && &p[12] < e; ) {

		/* Skip possible name in CNAME answer */
		if (*p != 0xc0) {
			while (*p && &p[12] < e)
				p++;
			p--;
		}

		type = htons(((uint16_t *)p)[1]);

		if (type == 5) 
		{
			/* CNAME answer. shift to the next section */
			dlen = htons(((uint16_t *) p)[5]);
			p += 12 + dlen;
		} 
		else if (type == DNS_A_RECORD) 
		{
			found = stop = 1;
		}
		else
		{
			stop = 1;
		}
	}

	if (found && &p[12] < e) 
	{
		dlen = htons(((uint16_t *) p)[5]);
		p += 12;

		if (p + dlen <= e) 
		{			
			query->addrlen = dlen;
			if (query->addrlen > sizeof(query->addr))
				query->addrlen = sizeof(query->addr);
			memcpy(query->addr, p, query->addrlen);
			
			dns->callback(context, query->name, query->addr, query->addrlen);
			rb_erase(&query->node, &dns->active);
			free(query);
			query = NULL;
		}
	}
}

void dns_poll(void *context, struct dns *dns)
{	
	int			n;
	struct sockaddr_in	sa;
	socklen_t		len = sizeof(sa);
	unsigned char		pkt[DNS_PACKET_LEN];
	

	while ((n = recvfrom(dns->sock, pkt, sizeof(pkt), 0, (struct sockaddr *) &sa, &len)) > 0 && n > (int) sizeof(struct header))
		parse_udp(context, dns, pkt, n);

}


/*

#include <sys/epoll.h>

#define MAX_EVENTS 3000

static int epfd;

int main(int argc, char *argv[])
{
	int i, rc;
	struct dns	*dns;

	char *host[7] = {"www.163.com", "www.pcpop.com", "www.blueidea.com", "www.androidin.net", "www.baidu.com", "www.cellphp.com", "www.im286.com"};
	
	struct epoll_event ev;
	struct epoll_event events[MAX_EVENTS];

	epfd = epoll_create(MAX_EVENTS);

	dns = dns_init();


	memset(&ev, 0, sizeof ev);

    ev.events  = EPOLLIN;
    ev.data.fd = dns->sock;
    epoll_ctl(epfd, EPOLL_CTL_ADD, dns->sock, &ev);
	

	for(i=0; i<7; i++)
	{
		dns_request(dns, host[i], DNS_A_RECORD);
	}

	for(i=0; i<7; i++)
	{
		dns_request(dns, host[i], DNS_A_RECORD);
	}
	
	for (;;)
	{
		int i, n;

		n = epoll_wait(epfd, events, MAX_EVENTS, -1);
		for (i = 0; i < n; i++) 
		{
			if(events[i].events & EPOLLIN)
				dns_poll(dns);
		}
	}
	
	return 0;
}
*/

