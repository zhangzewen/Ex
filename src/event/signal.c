#include "Signal.h"
#include "core/error.h"
int Sigaction(int signo, const struct sigaction *restrict act, struct sigaction *restrict oact)
{
	if(sigaction(signo, act, oact)  < 0 ) {
			err_sys("error");
	}
	return 0;
}
int Sigemptyset(sigset_t *set)
{
	if (sigemptyset(set) < 0)
	{
		err_sys("error");
	}
	return 0;
}
int Sigfillset(sigset_t *set);
int Sigaddset(sigset_t *set, int signum);
int Sigdelset(sigset_t *set, int signum);
int Sigismember(const sigset_t *set, int signum);
int Sigprocmask(int how, const sigset_t *set, sigset_t *oldset);
int Kill();
int Raise();