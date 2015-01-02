#ifndef _SYS_WAIT_H
#define _SYS_WAIT_H
#ifndef __TYPES_H
#include <types.h>
#endif

/* Bits in the third argument to `waitpid'.  */
#define WNOHANG 	1	/* Don't block waiting.  */
#define WUNTRACED	2	/* Report status of stopped children.  */

/* Everything extant so far uses these same bits.  */
/* If WIFEXITED(STATUS), the low-order 8 bits of the status.  */
#define WEXITSTATUS(status)	(((status) & 0xff00) >> 8)

/* If WIFSIGNALED(STATUS), the terminating signal.  */
#define WTERMSIG(status)	((status) & 0x7f)

/* If WIFSTOPPED(STATUS), the signal that stopped the child.  */
#define WSTOPSIG(status)	WEXITSTATUS(status)

/* Nonzero if STATUS indicates normal termination.  */
#define WIFEXITED(status)	(((status) & 0xff) == 0)

/* Nonzero if STATUS indicates termination by a signal.  */
#define WIFSIGNALED(status)	(((unsigned int)((status)-1) & 0xFFFF) < 0xFF)

/* Nonzero if STATUS indicates the child is stopped.  */
#define WIFSTOPPED(status)	(((status) & 0xff) == 0x7f)

/* Nonzero if STATUS indicates the child dumped core.  */
#define WCOREDUMP(status)	((status) & 0200)

/* Macros for constructing status values.  */
#define W_EXITCODE(ret, sig)	((ret) << 8 | (sig))
#define W_STOPCODE(sig) ((sig) << 8 | 0x7f)

/* Special values for the PID argument to `waitpid' and `wait4'.  */
#define WAIT_ANY	(-1)	/* Any process.  */
#define WAIT_MYPGRP	0	/* Any process in my process group.  */

extern pid_t waitpid __P((pid_t pid, int *__stat_loc, int options));

/* Wait for a child to die.  When one does, put its status in *STAT_LOC
   and return its process ID.  For errors, return (pid_t) -1.
 */
#define	wait(statloc)	waitpid(WAIT_ANY, statloc, 0)

#endif
