#include "defs.h"

int wdval;
int flags;
int peekc;
const char *comdiv;
int breakcnt;
int loopcnt;
BOOL execbrk;
int exitval;
int dolc;
const char **dolv;
/* FIXME */
DOLPTR argfor;
ARGPTR gchain;
IOPTR iotemp;
IOPTR iopend;
int ioset;
char *pidadr;
char *dolladr;
char *pcsadr;
char *exitadr;
char *cmdadr;
int wdset;
ARGPTR wdarg;
BOOL reserv;
int wdnum;
BOOL trapnote;
address end[1];		/* FIXME */
int serial;
BLKPTR stakbsy;
STKPTR stakbas;
STKPTR brkend;
STKPTR staktop;

#include <setjmp.h>
jmp_buf subshell;
jmp_buf errshell;

#if 0
/* FIXME */

int setjmp(jmp_buf bar)
{
}

void longjmp(jmp_buf bar, int foo)
{
}
#endif
