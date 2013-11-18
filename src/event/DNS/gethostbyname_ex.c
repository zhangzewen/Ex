#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>




int main(int argc, char *argv[])
{
	char *ptr;
	char **pptr;
	char str[INET_ADDRSTRLEN] = {0};

	struct hostent *hptr;

	while (--argc > 0) {
		ptr = *++argv;

		if ((hptr = gethostbyname(ptr)) == NULL) {
			fprintf(stderr, "gethostbyname error for host: %s: %s", ptr, hstrerror(h_errno));
			continue;
		}

		printf("offical hostname: %s\n", hptr->h_name);

		for (pptr = hptr->h_aliases; *pptr != NULL; pptr++) {
			printf("\talias: %s\n", *pptr);
		}

		switch (hptr->h_addrtype) {
			case AF_INET:
				pptr = hptr->h_addr_list;
				for (; *pptr != NULL; pptr++) {
					inet_ntop(hptr->h_addrtype, *pptr, str, sizeof(str));
					printf("\taddress: %s\n", str);
				}
				break;
			default:
				fprintf(stderr, "unknown address type");
				break;
		}
	}

	return 0;
}

