
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

static void do_method(const char *start, const char *end)
{
  char buff[BUFSIZ] = {0};
  strncpy(buff, start, end - start);

  printf("method = %s\n", buff);
}

static void do_path(const char *start, const char *end)
{
  char buff[BUFSIZ] = {0};
  strncpy(buff, start, end - start);

  printf("path = %s\n", buff);
}

static void do_version(const char *start, const char *end)
{
  char buff[BUFSIZ] = {0};
  strncpy(buff, start, end - start);

  printf("version = %s\n", buff);
}

static void do_kv(const char *key_start, const char *key_end, const char *value_start, const char *value_end)
{
  char buff[BUFSIZ] = {0};
  strncpy(buff, key_start, key_end - key_start);
  strncat(buff, ": ", strlen(": "));

  strncat(buff, value_start, value_end - value_start);
  
  printf("K/V -- >%s\n", buff);
}

int parse_http_request_line(http_request_t *r)
{
    char  ch;
    char  *p;
		char *tmp;
		int count = 0;
    enum {
        sw_start = 0,
        sw_method,
        sw_spaces_before_uri,
				sw_uri,
				sw_spaces_after_uri,
				sw_spaces_before_version,
				sw_version,
				sw_request_line_parse_almost_done,
				sw_request_line_parse_done,
				sw_almost_done,
				sw_done
    } state;

    state = (r->parse_state == -1)? sw_start: r->parse_state;

		for (p = r->buffer->pos; p != r->buffer->last; p++) {
			ch = *p;

			switch (state) {

				case sw_start:
					r->method_start = p;

					if (ch == '\r' || ch == '\n') {
						state = sw_request_line_parse_done;
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
						do_method(r->method_start, r->method_end);
						state = sw_spaces_before_uri;
						break;
					}

					if (ch < 'A' || ch > 'Z') {
						return -1;
					}

					break;

				case sw_spaces_before_uri:

					if (ch == '/') {
						r->path_start = p;
						state = sw_uri;
						break;
					}

				case sw_uri:
					if (ch == ' ') {
						state = sw_spaces_before_version;
						r->path_end = p;
						do_path(r->path_start, r->path_end);
					} 
					
					if (count > 1024) {
						printf("uri to large!\n");
						return -1;
					}

					count++;

					break;

				case sw_spaces_before_version:
					if (ch == 'H') {
						state = sw_version;
						r->version_start = p;
						break;
					}

				case sw_version:
					if (ch == '\r') {
						state = sw_request_line_parse_almost_done;
						r->version_end = p;
						do_version(r->version_start, r->version_end);
					}	

					break;
				case sw_request_line_parse_almost_done:
					if (ch == '\n') {
						state = sw_request_line_parse_done;
					}
					break;

				case sw_request_line_parse_done:
					if (ch == '\r') {
						tmp = p;
						state = sw_almost_done;
					} else {
						return -1;
					}
					break;
				case sw_almost_done:
					if (ch == '\n' && *tmp == '\r') {
						state = sw_done;
					} else {
						return -1;
					}
					break;
				case sw_done:
					break;

				default:
					break;

			}
			
		}

		r->parse_state = state;
		r->buffer->pos = p;

		return 0;
}

