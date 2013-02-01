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

void signal_handler(int signo, void (*handler)(int))
{
	struct sigaction act;
	
	memset(&act, '\0', sizeof(act));
	act.sa_handler = handler;
	act.sa_flags = 0;
	sigemptyset(&act.sa_mask);
	
	sigaction(signo, &act, NULL);
}
/* 
 * Soft:        Keepalived is a failover program for the LVS project
 *              <www.linuxvirtualserver.org>. It monitor & manipulate
 *              a loadbalanced server pool using multi-layer checks.
 * 
 * Part:        Signals framework.
 *  
 * Author:      Kevin Lindsay, <kevinl@netnation.com>
 *              Alexandre Cassen, <acassen@linux-vs.org>
 *              
 *              This program is distributed in the hope that it will be useful,
 *              but WITHOUT ANY WARRANTY; without even the implied warranty of
 *              MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *              See the GNU General Public License for more details.
 *
 *              This program is free software; you can redistribute it and/or
 *              modify it under the terms of the GNU General Public License
 *              as published by the Free Software Foundation; either version
 *              2 of the License, or (at your option) any later version.
 *
 * Copyright (C) 2001-2011 Alexandre Cassen, <acassen@linux-vs.org>
 */

#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <assert.h>


#include "signals.h"
#include "utils.h"

/* Local Vars */
void (*signal_SIGHUP_handler)(void *, int sig);
void *signal_SIGHUP_v;
void (*signal_SIGINT_handler)(void *, int sig);
void *signal_SIGINT_v;
void (*signal_SIGTERM_handler)(void *, int sig);
void *signal_SIGTERM_v;
void (*signal_SIGCHLD_handler)(void *, int sig);
void *signal_SIGCHLD_v;
void (*signal_SIGUSR1_handler)(void *, int sig);
void *signal_SIGUSR1_v;
/* for ignore signals */
void (*signal_SIGSEGV_handler)(void *, int sig);
void *signal_SIGSEGV_v;


static int signal_pipe[2] = { -1, -1 };

/* Local signal test */
int signal_pending(void)
{
	fd_set readset;
	int rc;
	struct timeval timeout = { 0, 0 };

	FD_ZERO(&readset);
	FD_SET(signal_pipe[0], &readset);

	rc = select(signal_pipe[0] + 1, &readset, NULL, NULL, &timeout);

	return rc > 0 ? 1 : 0;
}

/* Signal flag */
void signal_handler(int sig)
{
	if (write(signal_pipe[1], &sig, sizeof(int)) != sizeof(int))
	{
		DBG("signal_pipe write error %s", strerror(errno));
		assert(0);
	}
}

/* Signal wrapper */
void *signal_set(int signo, void (*func)(void *, int), void *v)
{
	int ret;
	struct sigaction sig;
	struct sigaction osig;

	sig.sa_handler = signal_handler;		/* signal_handler: general handler */
	sigemptyset(&sig.sa_mask);
	sig.sa_flags = 0;
#ifdef SA_RESTART
	sig.sa_flags |= SA_RESTART;
#endif /* SA_RESTART */

	ret = sigaction(signo, &sig, &osig);
	switch (signo)
	{
		case SIGHUP:
			signal_SIGHUP_handler = func;
			signal_SIGHUP_v = v;
			break;
		case SIGINT:
			signal_SIGINT_handler = func;
			signal_SIGINT_v = v;
			break;
		case SIGTERM:
			signal_SIGTERM_handler = func;
			signal_SIGTERM_v = v;
			break;
		case SIGCHLD:
			signal_SIGCHLD_handler = func;
			signal_SIGCHLD_v = v;
			break;
		/*  add by fanpf */
		case SIGUSR1:
			signal_SIGUSR1_handler = func;
			signal_SIGUSR1_v = v;
			break;

		case SIGSEGV:
			signal_SIGSEGV_handler = func;
			signal_SIGSEGV_v = v;
			break;
			
		case SIGPIPE:
			break;
			
	}

	if (ret < 0)
		return (SIG_ERR);
	else
		return (osig.sa_handler);
}

/* Signal Ignore */
void *signal_ignore(int signo)
{
	return signal_set(signo, NULL, NULL);
}

/* Handlers intialization */
void signal_handler_init(void)
{
	int n = pipe(signal_pipe);
	assert(!n);

	fcntl(signal_pipe[0], F_SETFL, O_NONBLOCK | fcntl(signal_pipe[0], F_GETFL));
	fcntl(signal_pipe[1], F_SETFL, O_NONBLOCK | fcntl(signal_pipe[1], F_GETFL));

	signal_SIGHUP_handler = NULL;
	signal_SIGINT_handler = NULL;
	signal_SIGTERM_handler = NULL;
	signal_SIGCHLD_handler = NULL;
	signal_SIGUSR1_handler = NULL;
	signal_SIGSEGV_handler = NULL;
}

void signal_wait_handlers(void)
{
	struct sigaction sig;

	sig.sa_handler = SIG_DFL;
	sigemptyset(&sig.sa_mask);
	sig.sa_flags = 0;

	/* Ensure no more pending signals */
	sigaction(SIGHUP, &sig, NULL);
	sigaction(SIGINT, &sig, NULL);
	sigaction(SIGTERM, &sig, NULL);
	sigaction(SIGCHLD, &sig, NULL);
	sigaction(SIGUSR1, &sig, NULL);
	sigaction(SIGPIPE, &sig, NULL);
	sigaction(SIGSEGV, &sig, NULL);

	/* reset */
	signal_SIGHUP_v = NULL;
	signal_SIGINT_v = NULL;
	signal_SIGTERM_v = NULL;
	signal_SIGCHLD_v = NULL;
	signal_SIGUSR1_v = NULL;
	signal_SIGSEGV_v = NULL;
}

void signal_reset(void)
{
	signal_wait_handlers();
	signal_SIGHUP_handler = NULL;
	signal_SIGINT_handler = NULL;
	signal_SIGTERM_handler = NULL;
	signal_SIGCHLD_handler = NULL;
	signal_SIGUSR1_handler = NULL;
	signal_SIGSEGV_handler = NULL;
}

void signal_handler_destroy(void)
{
	signal_wait_handlers();
	close(signal_pipe[1]);
	close(signal_pipe[0]);
	signal_pipe[1] = -1;
	signal_pipe[0] = -1;
}

int signal_rfd(void)
{
	return (signal_pipe[0]);
}

/* Handlers callback  */
void signal_run_callback(void)
{
	int sig;

	while (read(signal_pipe[0], &sig, sizeof(int)) == sizeof(int))
	{
		switch (sig)
		{
			case SIGHUP:
				if (signal_SIGHUP_handler)
					signal_SIGHUP_handler(signal_SIGHUP_v, SIGHUP);
				break;
			case SIGINT:
				if (signal_SIGINT_handler)
					signal_SIGINT_handler(signal_SIGINT_v, SIGINT);
				break;
			case SIGTERM:
				if (signal_SIGTERM_handler)
					signal_SIGTERM_handler(signal_SIGTERM_v, SIGTERM);
				break;
			case SIGCHLD:
				if (signal_SIGCHLD_handler)
					signal_SIGCHLD_handler(signal_SIGCHLD_v, SIGCHLD);
				break;
			case SIGUSR1:
				if (signal_SIGUSR1_handler)
					signal_SIGUSR1_handler(signal_SIGUSR1_v, SIGUSR1);
				break;

			case SIGSEGV:
				if (signal_SIGSEGV_handler)
					signal_SIGSEGV_handler(signal_SIGSEGV_v, SIGSEGV);
				break;
		
			default:
				break;
		}
	}
}
