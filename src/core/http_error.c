#ifndef _HTTP_ERROR_H_INCLUDED_
#define _HTTP_ERROR_H_INCLUDED_

#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>

#define BUFSIZE 4096
#if 0
static void log_message(int stauts, const char *mode, const char *fmt, va_list ap);
static void error_message(int status, const char *mode, const char *fmt, va_list ap);
#endif
void log_open(char *ident)
{
	openlog(ident, LOG_PID, LOG_DAEMON);
	return;
}

void log_close(void)
{
	closelog();
	return;
}

void log_message(int status, const char *mode, const char *fmt, va_list ap)
{
	char buf[BUFSIZE] = {0};
	vsprintf(buf, fmt, ap);
	
	if(errno == 0) {
		sprintf(buf + strlen(buf), "\n");
	} else {
		sprintf(buf + strlen(buf), ": %s\n", strerror(errno));
	}

	syslog(LOG_ERR, "%s: %s", mode, buf);
	if(status >= 0) {
		exit(status);
	}

	return;
}
__attribute__((unused))
static void error_message(int status , const char *mode, const char *fmt, va_list ap)
{
	char buf[BUFSIZ] = {0};
	char msg[BUFSIZ] = {0};
	
	vsprintf(buf, fmt, ap);
	
	if(errno == 0 || errno == ENOSYS) {
		snprintf(msg, sizeof(msg), "%s\n", buf);
	} else {
		snprintf(msg, sizeof(msg), "%s: %s\n", buf, strerror(errno));
	}

	fflush(stdout);
	fprintf(stderr, "%s: %s", mode, msg);

	if(status >= 0) {
		exit(status);
	}
}


void log_warning(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);

	log_message(-1, "warning", fmt, ap);
	
	va_end(ap);

	return ;
}

void log_fatal(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	log_message(1, "FATAL", fmt, ap);
	
	va_end(ap);
	return;
}


#endif
