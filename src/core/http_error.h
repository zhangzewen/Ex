#ifndef _HTTP_ERROR_H_INCLUDED_
#define _HTTP_ERROR_H_INCLUDED_

#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>

#define BUFSIZE 4096

void log_open(char *ident);

void log_close(void);

void log_message(int status, const char *mode, const char *fmt, va_list ap);

void error_message(int status , const char *mode, const char *fmt, va_list ap);


void log_warning(const char *fmt, ...);

void log_fatal(const char *fmt, ...);


#endif
