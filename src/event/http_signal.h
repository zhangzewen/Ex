#ifndef _HTTP_SIGNAL_H_INCLUDED
#define _HTTP_SIGNAL_H_INCLUDED
#include <signal.h>

extern int signal_pending(void);
extern void *signal_set(int signo, void (*func) (void *, int), void *);
extern void *signal_ignore(int signo);
extern void signal_handler_init(void);
extern void signal_handler_destroy(void);
extern void signal_reset(void);
extern void signal_run_callback(void);
extern void signal_wait_handlers(void);

extern int signal_rfd(void);

#endif
