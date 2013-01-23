#ifndef _HTTP_SIGNAL_H_INCLUDED
#define _HTTP_SIGNAL_H_INCLUDED

int Sigacton(int signo, const struct sigaction *restrict act, struct sigaction *restrict oact);
int Sigemptyset(sigset_t *set);
int Sigfillset(sigset_t *set);
int Sigaddset(sigset_t *set, int signum);
int Sigdelset(sigset_t *set, int signum);
int Sigismember(const sigset_t *set, int signum);
int Sigprocmask(int how, const sigset_t *set, sigset_t *oldset);
int Kill(pid_t pid, int sig);
int Raise(int sig);

int

#endif
