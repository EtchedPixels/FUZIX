#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <syscalls.h>

void abort(void)
{
	/* POSIX requires this sequence of behaviouir
		- unblock the signal
		- raise it
		- if we end up back here set the disposition to default
		- raise it again
	
	   If you take another signal in the middle of this and
	   change ABRT then we can end up at the _exit. POSIX has nothing
	   to say about what happens in such cases so you get to keep all
	   the pieces */
	sigrelse(SIGABRT);
	kill(getpid(),SIGABRT);		/* Correct one */
	signal(SIGABRT, SIG_DFL);
	kill(getpid(), SIGABRT);
	_exit(255);			/* WHAT!! */
}
