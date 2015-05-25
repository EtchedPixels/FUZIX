#ifndef __SETJMP_H
#define __SETJMP_H
#ifndef __TYPES_H
#include <types.h>
#endif

/* FIXME: need to add alt registers */
typedef int jmp_buf[7];

void longjmp __P((jmp_buf env, int rv));
#if defined(__SDCC_z80) || defined(__SDCC_z180)
int __setjmp __P((jmp_buf env));
#define setjmp(x)	__setjmp(x)
#else
int setjmp __P((jmp_buf env));
#endif

#endif
