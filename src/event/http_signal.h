#ifndef _HTTP_SIGNAL_H_INCLUDED
#define _HTTP_SIGNAL_H_INCLUDED
#include <signal.h>
int Sigacton(int signo, const struct sigaction *act, struct sigaction *oact);
int Sigemptyset(sigset_t *set);
int Sigfillset(sigset_t *set);
int Sigaddset(sigset_t *set, int signum);
int Sigdelset(sigset_t *set, int signum);
int Sigismember(const sigset_t *set, int signum);
int Sigprocmask(int how, const sigset_t *set, sigset_t *oldset);
int Kill(pid_t pid, int sig);
int Raise(int sig);

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
