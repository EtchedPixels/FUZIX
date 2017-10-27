/*
 * Z-80 assembler.
 * Header file, used by all
 * parts of the assembler.
 */
#include	<stdio.h>
#include	<string.h>
#include 	<stdlib.h>
#include	<stdint.h>
#include	<ctype.h>
#include	<setjmp.h>

/*
 * Table sizes, etc.
 */
#define	NCPS	8			/* # of characters in symbol */
#define	NHASH	64			/* # of hash buckets */
#define	HMASK	077			/* Mask for above */
#define	NFNAME	32			/* # of characters in filename */
#define	NERR	10			/* Size of error buffer */
#define	NCODE	128			/* # of characters in code buffer */
#define	NINPUT	128			/* # of characters in input line */
#define	NLPP	60			/* # of lines on a page */
#define NSEGMENT 4			/* # of segments */
#define	XXXX	0			/* Unused value */

/*
 * Exit codes.
 */
#define	GOOD	0
#define	BAD	1

/*
 * Listing modes.
 */
#define	NLIST	0			/* No list */
#define	ALIST	1			/* Address only */
#define	BLIST	2			/* Byte format */
#define	WLIST	3			/* Word format */
#define	SLIST	4			/* Source text only */

/*
 * Types. These are used
 * in both symbols and in address
 * descriptions. Observe the way the
 * symbol flags hide in the register
 * field of the address.
 */
#define	TMREG	0x007F			/* Register code */
#define	TMMDF	0x0001			/* Multidef */
#define	TMASG	0x0002			/* Defined by "=" */
#define	TMMODE	0xFF00			/* Mode */
#define	TMINDIR	0x8000			/* Indirect flag in mode */
#define TPUBLIC	0x0080			/* Exported symbol */

#define	TNEW	0x0000			/* Virgin */
#define	TUSER	0x0100			/* User name */
#define	TBR	0x0200			/* Byte register */
#define	TWR	0x0300			/* Word register */
#define	TSR	0x0400			/* Special register (I, R) */
#define	TDEFB	0x0500			/* defb */
#define	TDEFW	0x0600			/* defw */
#define	TDEFS	0x0700			/* defs */
#define	TDEFM	0x0800			/* defm */
#define	TORG	0x0900			/* org */
#define	TEQU	0x0A00			/* equ */
#define	TCOND	0x0B00			/* conditional */
#define	TENDC	0x0C00			/* end conditional */
#define	TNOP	0x0F00			/* nop */
#define	TRST	0x1000			/* restarts */
#define	TREL	0x1100			/* djnz, jr */
#define	TRET	0x1200			/* ret */
#define	TJMP	0x1300			/* call, jp */
#define	TPUSH	0x1400			/* push, pop */
#define	TIM	0x1500			/* im */
#define	TIO	0x1600			/* in, out */
#define	TBIT	0x1700			/* set, res, bit */
#define	TSHR	0x1800			/* sl, sr et al */
#define	TINC	0x1900			/* inc, dec */
#define	TEX	0x1A00			/* ex */
#define	TADD	0x1B00			/* add, adc, sbc */
#define	TLD	0x1C00			/* ld */
#define	TCC	0x1D00			/* condition code */
#define	TSUB	0x1E00			/* sub et al */
#define TSEGMENT 0x1F00			/* segments by number */
#define TEXPORT 0x2000			/* symbol export */

/*
 * Registers.
 */
#define	B	0
#define	C	1
#define	D	2
#define	E	3
#define	H	4
#define	L	5
#define	M	6
#define	A	7
#define	IX	8
#define	IY	9

#define	BC	0
#define	DE	1
#define	HL	2
#define	SP	3
#define	AF	4
#define	AFPRIME	5

#define	I	0
#define	R	1

/*
 * Condition codes.
 */
#define	CNZ	0
#define	CZ	1
#define	CNC	2
#define	CC	3
#define	CPO	4
#define	CPE	5
#define	CP	6
#define	CM	7

/*
 * Segments
 */
#define UNKNOWN		-1
#define ABSOLUTE	0
#define CODE		1
#define DATA		2
#define BSS		3

typedef	uint16_t	VALUE;		/* For symbol values */

/*
 * Address description.
 */
typedef	struct	ADDR	{
	int	a_type;			/* Type */
	VALUE	a_value;		/* Index offset, etc */
	int	a_segment;		/* Segment relative to */
	struct SYM *a_sym;		/* Symbol tied to this address */
					/* NULL indicates simple own segment */
}	ADDR;

/*
 * Symbol.
 */
typedef	struct	SYM	{
	struct	SYM *s_fp;		/* Link in hash */
	char	s_id[NCPS];		/* Name */
	int	s_type;			/* Type */
	VALUE	s_value;		/* Value */
	int	s_segment;		/* Segment this symbol is relative to */
	uint16_t s_number;		/* Symbol number 1..n */
}	SYM;

/*
 * External variables.
 */
extern	char	*cp;
extern	char	*ep;
extern	char	*ip;
extern	char	cb[];
extern	char	eb[];
extern	char	ib[];
extern	FILE	*ifp;
extern	FILE	*ofp;
extern	FILE	*lfp;
extern	int	line;
extern	int	lmode;
extern	VALUE	laddr;
extern	SYM	sym[];
extern	int	pass;
extern	SYM	*phash[];
extern	SYM	*uhash[];
extern	int	lflag;
extern	jmp_buf	env;
extern	VALUE	dot[NSEGMENT];
extern  int	segment;
extern	int	debug_write;

extern void asmline(void);
extern void asmld(void);
extern ADDR *getldaddr(ADDR *, int *, int *, ADDR *);
extern void outop(int, ADDR *);
extern void comma(void);
extern void istuser(ADDR *);
extern int ccfetch(ADDR *);
extern int symhash(char *);
extern void err(char);
extern void uerr(char *);
extern void aerr(void);
extern void qerr(void);
extern void storerror(int);
extern void getid(char *, int);
extern SYM *lookup(char *, SYM *[], int);
extern int symeq(char *, char *);
extern void symcopy(char *, char *);
extern int get(void);
extern int getnb(void);
extern void unget(int);
extern void getaddr(ADDR *);
extern void expr1(ADDR *, int, int);
extern void expr2(ADDR *);
extern void expr3(ADDR *, int);
extern void isokaors(ADDR *, int);
extern void outpass(void);
extern void outabsolute(int);
extern void outsegment(int);
extern void outaw(uint16_t);
extern void outab(uint8_t);
extern void outraw(ADDR *);
extern void outrab(ADDR *);
extern void outeof(void);
extern void outbyte(uint8_t);
extern void outflush(void);
extern void list(void);
extern void list1(char *, int, int);
extern void syminit(void);
