int wdval;
int iopend;
int flags;
int peekc;
char *comdiv;
int breakcnt;
int loopcnt;
int execbrk;
int exitval;
int dolc;
char **dolv;
/* FIXME */
void *argfor;
void *gchain;
int iotemp;
int iopend;
int ioset;
int output;
char *pidadr;
char *dolladr;
char *pcsadr;
char *pidadr;
char *exitadr;
char *cmdadr;
int wdset;
void *wdarg;
int reserv;
int wdnum;
int trapnote;
int end;
int serial;

#include <setjmp.h>
jmp_buf subshell;
jmp_buf errshell;

/* FIXME */

int setjmp(jmp_buf bar) {}
void longjmp(jmp_buf bar, int foo) {}