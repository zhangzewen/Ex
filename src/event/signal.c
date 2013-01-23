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
int Sigfillset(sigset_t *set)
{
	if(sigfillset(set) < 0)
	{
		err_sys("error");
	}
	return 0;
}
int Sigaddset(sigset_t *set, int signum)
{
	if(sigaddset(set, signum) < 0)
	{
		err_sys("error");
	}
	return 0;
}
int Sigdelset(sigset_t *set, int signum)
{
	if(sigdelset(set, signum) < 0)
	{
		err_sys("error");
	}
	return 0;
}
int Sigismember(const sigset_t *set, int signum)
{
	int ret = 0;
	ret = sigismember(set, signum);
	if(ret == 1)
	{
		printf("%d is a member of set", signum);
		return 1;
	}else if (ret == 0)
	{
		printf("%d is not a member of set", signum);
		return 0;
	}else{
		err_sys("error");
	}
	return -1;
}
int Sigprocmask(int how, const sigset_t *set, sigset_t *oldset)
{
	if(sigprocmask(how, set, oldset) < 0)
	{
		err_sys("error");
	}
	return 0;
}
int Kill(pid_t pid, int sig)
{
	if(kill(pid, sig) < 0)
	{
		err_sys("error");
	}
	return 0;
}
int Raise(int sig)
{
	if(raise(sig) != 0)
	{
		err_sys("error");
	}
	return 0;
}
