#ifndef __SETJMP_H
#define __SETJMP_H
#ifndef __TYPES_H
#include <types.h>
#endif

#if defined(__SDCC_z80) || defined(__SDCC_z1280)

	/* FIXME: need to add alt registers */
	typedef int jmp_buf[7];
	extern int __setjmp(jmp_buf __env);
	#define setjmp(x) __setjmp(x)

#elif defined(__CC65__)

	typedef char jmp_buf[5];
	extern int _setjmp(jmp_buf __env);
	#define setjmp(x) _setjmp(x)

#elif defined(__MSP430__)

	typedef uint16_t jmp_buf[11];
	extern int setjmp(jmp_buf __env);
	__attribute__((__noreturn__)) void longjmp (jmp_buf env, int val);

#elif defined(__m6809__)

	typedef uint16_t jmp_buf[4];
	extern int setjmp(jmp_buf __env);
	__attribute__((__noreturn__)) void longjmp (jmp_buf env, int val);

#elif defined(__mc68000__)

	typedef uint32_t jmp_buf[13];	/* A2-A7/D2-D7/return */
	extern int setjmp(jmp_buf __env);
	__attribute__((__noreturn__)) void longjmp (jmp_buf env, int val);

#elif defined(__pdp11__)

	typedef uint16_t jmp_buf[5];
	extern int setjmp(jmp_buf __env);
	__attribute__((__noreturn__)) void longjmp (jmp_buf env, int val);

#else
	#error jmp_buf definition not set for this architecture
#endif

extern void longjmp(jmp_buf __env, int __rv);

#endif
