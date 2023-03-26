/*
 *	abort(): unblock SIGABRT, send it and then if that doesn't do
 *	anything set it to default and send it again - which will.
 *
 *	Does not fflush anything as POSIX does not require that.
 */
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <syscalls.h>

void abort(void)
{
	sigrelse(SIGABRT);
	signal(SIGABRT, SIG_DFL);
	kill(getpid(),SIGABRT);		/* Correct one */
	signal(SIGKILL, SIG_DFL);
	kill(SIGABRT, getpid());
	_exit(255);			/* WHAT!! */
}
