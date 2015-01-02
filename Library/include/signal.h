#ifndef __SIGNAL_H
#define __SIGNAL_H
#ifndef __TYPES_H
#include <types.h>
#endif

#define NSIGS	  32      /* Number of signals <= 32 */

#define SIGHUP		 1
#define SIGINT		 2
#define SIGQUIT		 3
#define SIGILL		 4
#define SIGTRAP		 5
#define SIGABRT		 6
#define SIGIOT		 6
#define SIGBUS		 7
#define SIGFPE		 8
#define SIGKILL		 9
#define SIGUSR1		10
#define SIGSEGV		11
#define SIGUSR2		12
#define SIGPIPE		13
#define SIGALRM		14
#define SIGTERM		15
#define SIGSTKFLT	16
#define SIGCHLD		17
#define SIGCONT		18
#define SIGSTOP		19
#define SIGTSTP		20
#define SIGTTIN		21
#define SIGTTOU		22
#define SIGURG		23
#define SIGXCPU		24
#define SIGXFSZ		25
#define SIGVTALRM	26
#define SIGPROF		27
#define SIGWINCH	28
#define SIGIO		29
#define SIGPOLL		SIGIO
#define SIGPWR		30
#define SIGSYS		31
#define	SIGUNUSED	31

/* signals values */
typedef enum {
	__NOTASIGNAL = 0,
	_SIGLAST = 30000
} signal_t;

#define sigmask(sig) (1UL<<((sig)-1)) 	/* signal mask */

typedef uint32_t sigset_t;	/* at least 16 bits: use 32 in user space */
				/* for expansion space */
/* Type of a signal handler.  */
typedef void (*sighandler_t) __P((int));

#define SIG_DFL ((sighandler_t)0)	/* default signal handling */
#define SIG_IGN ((sighandler_t)1)	/* ignore signal */
#define SIG_ERR ((sighandler_t)-1)	/* error return from signal */

extern char *sys_siglist[];

extern void sighold(int sig);
extern void sigrelse(int sig);
extern void sigignore(int sig);
extern sighandler_t sigset(int sig, sighandler_t disp);

typedef int sig_atomic_t;

#endif
