#ifndef __SETJMP_H
#define __SETJMP_H
#ifndef __TYPES_H
#include <types.h>
#endif

#if defined(__8085__)

	typedef unsigned jmp_buf[3];	/*  BC (sp) and sp */
	extern int _setjmp(jmp_buf __env);
	#define setjmp(x) _setjmp(x)

#elif defined(__8080__)

	typedef unsigned jmp_buf[3];	/*  BC (sp) and sp */
	extern int _setjmp(jmp_buf __env);
	#define setjmp(x) _setjmp(x)

#elif defined(__z80__)

	typedef unsigned jmp_buf[5];	/*  IY IX BC (sp) and sp */
	extern int _setjmp(jmp_buf __env);
	#define setjmp(x) _setjmp(x)

#elif defined(__z8__)

	typedef unsigned jmp_buf[6];	/*  R4-R11 (sp) and sp */
	extern int _setjmp(jmp_buf __env);
	#define setjmp(x) _setjmp(x)

#elif defined(__super8__)

	typedef unsigned jmp_buf[6];	/*  R4-R11 (sp) and sp */
	extern int _setjmp(jmp_buf __env);
	#define setjmp(x) _setjmp(x)

#elif defined(__65c816__)

	typedef unsigned jmp_buf[2];	/*  (SP) and Y */
	extern int _setjmp(jmp_buf __env);
	#define setjmp(x) _setjmp(x)

#elif defined(__CC65__)

	typedef char jmp_buf[5];
	extern int _setjmp(jmp_buf __env);
	#define setjmp(x) _setjmp(x)

#elif defined(__CC68__)

	typedef char jmp_buf[4];
	extern int _setjmp(jmp_buf __env);
	#define setjmp(x) _setjmp(x)

#elif defined(__CC9995__)

	typedef unsigned int jmp_buf[13];
	extern int _setjmp(jmp_buf __env);
	#define setjmp(x) _setjmp(x)

#elif defined(__MSP430__)

	typedef uint16_t jmp_buf[11];
	extern int setjmp(jmp_buf __env);
	__attribute__((__noreturn__)) void longjmp (jmp_buf __env, int __val);

#elif defined(__m6809__)

	typedef uint16_t jmp_buf[4];
	extern int setjmp(jmp_buf __env);
	__attribute__((__noreturn__)) void longjmp (jmp_buf __env, int __val);

#elif defined(mc68hc11)

	/* Quite large due to the dp fake registers */
	typedef uint16_t jmp_buf[32];
	extern int setjmp(jmp_buf __env);
	__attribute__((__noreturn__)) void longjmp (jmp_buf __env, int __val);

#elif defined(__mc68000__)

	typedef uint32_t jmp_buf[13];	/* A2-A7/D2-D7/return */
	extern int setjmp(jmp_buf __env);
	__attribute__((__noreturn__)) void longjmp (jmp_buf __env, int __val);

#elif defined(__pdp11__)

	typedef uint16_t jmp_buf[5];
	extern int setjmp(jmp_buf __env);
	__attribute__((__noreturn__)) void longjmp (jmp_buf __env, int __val);

#elif defined(__XTENSA_CALL0_ABI__)

    typedef uint32_t jmp_buf[6];
    extern int setjmp(jmp_buf __env);
	__attribute__((__noreturn__)) void longjmp (jmp_buf __env, int __val);

#elif defined(__XTENSA_WINDOWED_ABI__)

	/* Fetch the compiler's setjmp.h. */
	#include_next <setjmp.h>
	
#elif defined(__ARM_EABI__)

	typedef uint32_t jmp_buf[10];
	extern int setjmp(jmp_buf __env);
	__attribute__((__noreturn__)) void longjmp (jmp_buf __env, int __val);

#elif defined(__ns32k__)

	typedef uint32_t jmp_buf[13];
	extern int setjmp(jmp_buf __env);
	__attribute__((__noreturn__)) void longjmp (jmp_buf __env, int __val);

#elif defined(__riscv)

	typedef uint64_t jmp_buf[19];
	extern int setjmp(jmp_buf __env);
	__attribute__((__noreturn__)) void longjmp (jmp_buf __env, int __val);

#elif defined(__i86)

	typedef uint16_t jmp_buf[12];
	extern int setjmp(jmp_buf __env);
	__attribute__((__noreturn__)) void longjmp (jmp_buf __env, int __val);

#elif defined(__ee200__)

	typedef uint16_t jmp_buf[6];
	extern int setjmp(jmp_buf __env);
	#define setjmp(x) _setjmp(x)

#else
	#error jmp_buf definition not set for this architecture
#endif

extern void longjmp(jmp_buf __env, int __rv);

#endif
