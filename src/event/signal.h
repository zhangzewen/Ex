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

void signal_handler(int signo, void (*handler)(void));

#endif
/* 
 * Soft:        Keepalived is a failover program for the LVS project
 *              <www.linuxvirtualserver.org>. It monitor & manipulate
 *              a loadbalanced server pool using multi-layer checks.
 * 
 * Part:        signals.c include file.
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

#ifndef _SIGNALS_H
#define _SIGNALS_H

/* Prototypes */
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
