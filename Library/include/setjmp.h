#ifndef __SETJMP_H
#define __SETJMP_H
#ifndef __TYPES_H
#include <types.h>
#endif

/* FIXME: need to add alt registers */
typedef int jmp_buf[7];

int setjmp __P((jmp_buf env));
void longjmp __P((jmp_buf env, int rv));

#endif
