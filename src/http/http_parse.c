
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <string.h>
#include "http_parse.h"
#include "Define_Macro.h"
#include <string.h>
#include "http_buffer.h"


int parse_http_request_line(http_request_t *r, http_buffer_t *buff)
{
    char  ch;
    char  *p;
    enum {
        sw_start = 0,
        sw_method,
        sw_spaces_before_uri,
				sw_spaces_after_uri,
				sw_spaces_before_version,
				sw_uri,
				sw_version,
				sw_almost_done,
				sw_done
    } state;

    state = sw_start;

    for (p = buff->pos; p != buff->end; p++) {
        ch = *p;

        switch (state) {

        case sw_start:
            r->method_start = p;

            if (ch == '\r' || ch == '\n') {
                break;
            }

            if (ch < 'A' || ch > 'Z') {
                return -1;
            }

            state = sw_method;
            break;

        case sw_method:
            if (ch == ' ') {
                r->method_end = p;

                state = sw_spaces_before_uri;
                break;
            }

            if (ch < 'A' || ch > 'Z') {
                return -1;
            }

            break;

        case sw_spaces_before_uri:

            if (ch == '/') {
                r->uri_start = p;
                state = sw_uri;
                break;
            }

				case sw_uri:
						if (ch == ' ') {
							state = sw_sapces_before_version;
							r->uri_end = p;
						} 

						break;
				case sw_spaces_before_version:
						if (ch == 'H') {
							state = sw_version;
							r->version_start = p;
							break;
						}
						
				case sw_version:
						if (ch == '\r') {
							state = sw_almost_done;
							r->version_end = p;
						}	

						break;
				case sw_almost_done:
						if (ch == '\n') {
							state = sw_done;
						}
						break;
				case sw_done:
						break;

				default:
						break;

        }
    }

    buff->pos = p;

    return 0;
}


