#ifndef __SIGNAL_H
#define __SIGNAL_H
#ifndef __TYPES_H
#include <types.h>
#endif

#define NSIGS		16	/* Number of signals <= 16 */

#define SIGHUP  	1	/* hangup */
#define SIGINT  	2	/* interrupt */
#define SIGQUIT 	3	/* quit */
#define SIGILL  	4	/* illegal instruction (not reset when caught */
#define SIGTRAP 	5	/* trace trap (not reset when caught) */
#define SIGABRT  	6	/* abort */
#define SIGIOT  	6	/* IOT instruction */
#define SIGEMT  	7	/* EMT instruction */
#define SIGFPE  	8	/* floating point exception */
#define SIGKILL 	9	/* kill */
#define SIGBUS  	10	/* bus error */
#define SIGSEGV 	11	/* segmentation violation */
#define SIGSYS  	12	/* bad argument to system call */
#define SIGPIPE 	13	/* write on a pipe with no one to read it */
#define SIGALRM 	14	/* alarm clock */
#define SIGTERM 	15	/* software termination signal from kill */

/* signals values */
typedef enum {
	__NOTASIGNAL = 0,
	_SIGLAST = 30000
} signal_t;

#define sigmask(sig) (1UL<<((sig)-1)) 	/* signal mask */

typedef uint32_t sigset_t;	/* at least 16 bits: use 32 in user space */
				/* for expansion space */
/* Type of a signal handler.  */
typedef void (*sighandler_t)(int);

#define SIG_DFL ((sighandler_t)0)	/* default signal handling */
#define SIG_IGN ((sighandler_t)1)	/* ignore signal */

extern char *sys_siglist[];

extern void sighold(int sig);
extern void sigrelse(int sig);
extern void sigignore(int sig);
extern sighandler_t sigset(int sig, sighandler_t disp);

#endif
