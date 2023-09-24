/*
 *	Load formats: a.out for bigger stuff, our own for little micros
 */
 
#ifndef _SYS_EXEC_H
#define _SYS_EXEC_H

#if defined(__mc68000__) || defined(__ns32k__) || defined(__ARM_ARCH_7EM__) || defined(__riscv)

#include "a.out.h"

/* Feature bits for 32bit to go with the 32bit exec header once done */
#define AF_68000_020	1		/* Has 68020 features */
#else

/* 16 byte header for current style binary. We try to reflect the general
   pattern of naming in classic Unixlike systems */

/* Do not change these values. They are (or will be) shared with asm stubs
   and the toolchain */
struct exec {
	uint16_t a_magic;
#define EXEC_MAGIC	0x80A8		/* Just need something to id with */
	uint8_t a_cpu;
#define A_8080		1		/* 8080 series processors (and Z80) */
#define A_6800		2		/* 6800 series */
#define A_6502		3		/* 6502 */
#define A_6809		4		/* 6809 */
#define A_RABBIT	5		/* Rabbit is unZ80 enough to be its own */
#define A_MSP340	6		/* MSP340 series */
#define A_PDP11		7		/* PDP-11 */
#define A_8086		8		/* x86 16bit */
#define A_68000		9		/* 68000 32bit */
#define A_NS32K		10		/* NS32K 32bit */
#define A_LX106		11		/* lx106 32bit */
#define A_ARM		12		/* ARM 32bit */
#define A_LX6  		13		/* lx6 32bit */
	uint8_t a_cpufeat;
#define AF_8080_8085	1		/* Uses 8085 instructions */
#define AF_8080_Z80	2		/* Uses legal Z80 instructions */
#define AF_8080_Z180	4		/* Uses Z180 instructions */
#define AF_8080_Z280	8		/* Uses Z280 instructions */
#define AF_8080_EZ80	16		/* Uses EZ80 extensions */

#define AF_6800_6803	1		/* PSHX etc */
#define AF_6800_6303	2		/* XGDX and similar */
#define AF_6800_68HC11	4		/* Y register and other extensions */

#define AF_6C502	1		/* CMOS 6502 extra instructions */
#define AF_65C816	2		/* Uses 65C816 instructions */
#define AF_65C816_B0	4		/* 65C816 assuming bank 0 (so can use
					   stack relative addressing) */
#define AF_65C02_BITOP	8		/* SMB/RMB/TSB/TRB/BBR */

#define AF_6809_6309	1		/* 6309 extensions */

#define AF_RABBIT_R3K	1		/* Uses Rabbit 3000 additions */

#define AF_LX106_ESP8266 1      	/* ESP8266 module */

#define AF_LX6_ESP32     1      	/* ESP32 module */

#define AF_CORTEX_M0    1       	/* Cortex M0 ARM */

	uint8_t a_base;			/* Load address page */
	uint8_t a_hints;
#define HINT_GRAPHICS	1		/* Hint that this binary uses graphics */
#define HINT_DEBUG	2		/* Debug data follows binary image */
	uint16_t a_text;
	uint16_t a_data;
	uint16_t a_bss;
	uint8_t a_entry;		/* Entry point - 0-255 bytes in only */
	/* These are kept in pages */
	uint8_t a_size;			/* Binary memory request 0 = all */
	uint8_t a_stack;		/* Stack size hint (not yet used) */
	uint8_t a_zp;			/* Zero/Direct page space required */

	/* This isn't really part of the header but a location fixed after
	   it */
	/* uint16_t a_sigvec; */
};


#endif
#endif

