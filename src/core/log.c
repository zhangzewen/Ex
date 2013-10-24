#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "log.h"
#include <errno.h>
static void _warn_helper(int severity, int log_errno, const char *fmt, va_list ap);
static void event_log(int severity, const char *msg);

void event_err(int eval, const char *fmt, ...)
{
	va_list ap;
	
	va_start(ap, fmt);
	_warn_helper(_EVENT_LOG_ERR, errno, fmt, ap);
	va_end(ap);
	exit(eval);
}


void event_warn(const char *fmt, ...)
{
	va_list ap;
	
	va_start(ap, fmt);
	
	_warn_helper(_EVENT_LOG_WARN, errno, fmt, ap);
	
	va_end(ap);
}

void event_errx(int eval, const char *fmt, ...)

{
	va_list ap;
	
	va_start(ap, fmt);
	
	_warn_helper(_EVENT_LOG_ERR, -1, fmt, ap);
	va_end(ap);
	exit(eval);
}


void event_warnx(const char *fmt, ...)
{
	va_list ap;
	
	va_start(ap, fmt);
	
	_warn_helper(_EVENT_LOG_WARN, -1, fmt, ap);
	va_end(ap);
}

void event_msgx(const char *fmt, ...)
{
	va_list ap;
	
	va_start(ap, fmt);
	
	_warn_helper(_EVENT_LOG_MSG, -1, fmt, ap);
	
	va_end(ap);
}


void _event_debugx(const char *fmt, ...)
{
	va_list ap;
	
	va_start(ap, fmt);
	
	_warn_helper(_EVENT_LOG_DEBUG, -1, fmt, ap);
	
	va_end(ap);	
}


static void _warn_helper(int severity, int log_errno, const char *fmt, va_list ap)
{
	char buf[BUFSIZ] = {0};
	size_t len;
	
	if (fmt != NULL) {
		vsnprintf(buf, sizeof(buf), fmt, ap);
	}else{
		buf[0] = '\0';
	}


	if (log_errno >=0) {
		len = strlen(buf);
		
		if (len < sizeof(buf) - 1) {
			
			snprintf(buf + len, sizeof(buf) - len, ":%s", strerror(log_errno));	
			
			buf[sizeof(buf) - len] = '\0';
		}
	}

	event_log(severity, buf);
}


static void event_log(int severity, const char *msg)
{
	const char *severity_str;
	switch (severity) {
		case _EVENT_LOG_DEBUG:
			severity_str = "debug";
			break;
		case _EVENT_LOG_MSG:
			severity_str = "msg";
			break;
		case _EVENT_LOG_WARN:
			severity_str = "err";
			break;
		case _EVENT_LOG_ERR:
			break;
			severity_str = "err";
		default:
			severity_str = "???";
			break;
	}
	fprintf(stderr, "[%s] %s\n", severity_str, msg);
}
