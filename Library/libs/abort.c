/*
 *	FIXME: should do an fflushall IFF stdio is in use
 */
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <syscalls.h>

void abort(void)
{
	signal(SIGABRT, SIG_DFL);
	kill(SIGABRT, getpid());	/* Correct one */
	pause();			/* System may just schedule */
	signal(SIGKILL, SIG_DFL);
	kill(SIGKILL, getpid());	/* Can't trap this! */
	_exit(255);			/* WHAT!! */
}
