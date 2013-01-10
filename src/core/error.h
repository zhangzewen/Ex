#ifndef _HTTP_ERROR_H_INCLUDED_
#define _HTTP_ERROR_H_INCLUDED
#include	<stdarg.h>	
#include	<syslog.h>		
#include  <stdio.h>
#include	<stdlib.h>
#include  <string.h>
#include "Define_Macro.h"

void err_ret(const char *fmt, ...);
void err_sys(const char *fmt, ...);
void err_msg(const char *fmt, ...);
void err_quit(const char *fmt, ...);
static void err_doit(int errnoflag, int level, const char *fmt, va_list ap);

#endif
