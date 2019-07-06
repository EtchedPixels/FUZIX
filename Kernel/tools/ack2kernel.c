/*
 * Ack2kernel based upon aelod
 *
 * Simple tool to produce a memory dump for an absolute
 * ack.out file. Suitable for noddy operating systems
 * like DOS, CP/M, Arthur, etc. Also useful for RAM
 * images (but *not* ROM images, note) and, more
 * importantly, general test purposes.
 * 
 * Mostly pinched from the ARM cv (and then rewritten in
 * ANSI C). Which, according to the comment, was pinched
 * from m68k2; therefore I am merely continuing a time-
 * honoured tradition.
 * 
 * (I was 10 when the original for this was checked into
 * CVS...)
 * 
 * dtrg, 2006-10-17

Copyright (c) 1987, 1990, 1993, 2005 Vrije Universiteit, Amsterdam, The Netherlands.
All rights reserved.

Redistribution and use of the Amsterdam Compiler Kit in source and
binary forms, with or without modification, are permitted provided
that the following conditions are met:

   * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.

   * Redistributions in binary form must reproduce the above
     copyright notice, this list of conditions and the following
     disclaimer in the documentation and/or other materials provided
     with the distribution.

   * Neither the name of Vrije Universiteit nor the names of the
     software authors or contributors may be used to endorse or
     promote products derived from this software without specific
     prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS, AUTHORS, AND
CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL VRIJE UNIVERSITEIT OR ANY AUTHORS OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <inttypes.h>
#include <unistd.h>
#include <sys/stat.h>

/*
 * output format for ACK assemblers
 */

struct outhead {
	uint16_t oh_magic;	/* magic number */
	uint16_t oh_stamp;	/* version stamp */
	uint16_t oh_flags;	/* several format flags */
	uint16_t oh_nsect;	/* number of outsect structures */
	uint16_t oh_nrelo;	/* number of outrelo structures */
	uint16_t oh_nname;	/* number of outname structures */
	uint32_t oh_nemit;		/* sum of all os_flen */
	uint32_t oh_nchar;		/* size of string area */
};

#define O_MAGIC	0x0202			/* magic number of output file */
#define	O_STAMP	0			/* version stamp */
#define MAXSECT	64			/* Maximum number of sections */

#define	HF_LINK	0x0004			/* unresolved references left */
#define	HF_8086	0x0008			/* os_base specially encoded */

struct outsect {
	uint32_t os_base;		/* startaddress in machine */
	uint32_t os_size;		/* section size in machine */
	uint32_t os_foff;		/* startaddress in file */
	uint32_t os_flen;		/* section size in file */
	uint32_t os_lign;		/* section alignment */
};

struct outrelo {
	uint16_t or_type;		/* type of reference */
	uint16_t or_sect;		/* referencing section */
	uint16_t or_nami;       /* referenced symbol index */
	uint32_t or_addr;		/* referencing address */
};

struct outname {
	union {
	  char	*on_ptr;		/* symbol name (in core) */
	  long	on_off;			/* symbol name (in file) */
	}	on_u;
#define on_mptr	on_u.on_ptr
#define on_foff	on_u.on_off
	uint16_t on_type;		/* symbol type */
	uint16_t on_desc;		/* debug info */
	uint32_t on_valu;		/* symbol value */
};

/*
 * relocation type bits
 */
#define RELSZ   0x0fff          /* relocation length */
#define RELO1        1          /* 1 byte */
#define RELO2        2          /* 2 bytes */
#define RELO4        3          /* 4 bytes */
#define RELOPPC      4          /* PowerPC 26-bit address */
#define RELOPPC_LIS  5          /* PowerPC lis */
#define RELOVC4      6          /* VideoCore IV address in 32-bit instruction */
#define RELOMIPS     7          /* MIPS */
#define RELO2HI      8          /* high 2 bytes of word */
#define RELO2HISAD   9          /* high 2 bytes of word, sign adjusted */

#define RELPC	0x2000			/* pc relative */
#define RELBR	0x4000			/* High order byte lowest address. */
#define RELWR	0x8000			/* High order word lowest address. */

/*
 * section type bits and fields
 */
#define S_TYP	0x007F			/* undefined, absolute or relative */
#define S_EXT	0x0080			/* external flag */
#define S_ETC	0x7F00			/* for symbolic debug, bypassing 'as' */

/*
 * S_TYP field values
 */
#define S_UND	0x0000			/* undefined item */
#define S_ABS	0x0001			/* absolute item */
#define S_MIN	0x0002			/* first user section */
#define S_MAX	(S_TYP-1)		/* last user section */
#define S_CRS	S_TYP			/* on_valu is symbol index which contains value */

/*
 * S_ETC field values
 */
#define S_SCT	0x0100			/* section names */
#define S_LIN	0x0200			/* hll source line item */
#define S_FIL	0x0300			/* hll source file item */
#define S_MOD	0x0400			/* ass source file item */
#define S_COM	0x1000			/* Common name. */
#define S_STB	0xe000			/* entries with any of these bits set are
					   reserved for debuggers
					*/

/*
 * structure sizes on disk (bytes in file; add digits in SF_*)
 * Note! These are NOT the sizes in memory (64-bit architectures will have
 * a different layout).
 */
#define SZ_HEAD		20
#define SZ_SECT		20
#define SZ_RELO		10
#define SZ_NAME		12

/*
 * file access macros
 */
#define BADMAGIC(x)	((x).oh_magic!=O_MAGIC)
#define OFF_SECT(x)	SZ_HEAD
#define OFF_EMIT(x)	(OFF_SECT(x) + ((long)(x).oh_nsect * SZ_SECT))
#define OFF_RELO(x)	(OFF_EMIT(x) + (x).oh_nemit)
#define OFF_NAME(x)	(OFF_RELO(x) + ((long)(x).oh_nrelo * SZ_RELO))
#define OFF_CHAR(x)	(OFF_NAME(x) + ((long)(x).oh_nname * SZ_NAME))


/*
 * Header and section table of ack object file.
 */
 
struct outhead outhead;
struct outsect outsect[S_MAX];
char* stringarea;

char* outputfile = NULL;                /* Name of output file, or NULL */
char* program;                          /* Name of current program: argv[0] */

FILE* input;                            /* Input stream */
FILE* output;                           /* Output stream */

#define readf(a, b, c)	fread((a), (b), (int)(c), input)
#define writef(a, b, c)	fwrite((a), (b), (int)(c), output)

bool verbose = false;

/*
 *	Kernel segments. We stack common and discard over the BSS space
 *	and unpack them when running.
 */
enum {
	TEXT = 0,
	ROM,
	DATA,
	BSS,
	COMMON,
	DISCARD,
	NUM_SEGMENTS
};

static const char *names[] = {
	"text",
	"rom",
	"data",
	"bss",
	"common",
	"discard"
};

/* Produce an error message and exit. */

void fatal(const char* s, ...)
{
	va_list ap;
	
	fprintf(stderr, "%s: ",program) ;
	
	va_start(ap, s);
	vfprintf(stderr, s, ap);
	va_end(ap);
	
	fprintf(stderr, "\n");
	
	if (outputfile)
		unlink(outputfile);
	exit(1);
}

/* Calculate the result of a aligned to b (rounding up if necessary). */

long align(long a, long b)
{
	a += b - 1;
	return a - a % b;
}
 
int follows(struct outsect* pa, struct outsect* pb)
{
	/* return 1 if pa follows pb */
 
	return (pa->os_base == align(pb->os_base+pb->os_size, pa->os_lign));
}

/* Copies the contents of a section from the input stream
 * to the output stream, zero filling any uninitialised
 * space. */
 
void emits(struct outsect* section)
{
	char buffer[BUFSIZ];

	/* Copy the actual data. */
	
	{
		long n = section->os_flen;
		while (n > 0)
		{
			int blocksize = (n > BUFSIZ) ? BUFSIZ : n;
			readf(buffer, 1, blocksize);
			writef(buffer, 1, blocksize);
			n -= blocksize;
		}
	}
	
	/* Zero fill any remaining space. */
	
	if (section->os_flen != section->os_size)
	{
		long n = section->os_size - section->os_flen;
		memset(buffer, 0, BUFSIZ);

		while (n > 0)
		{
			int blocksize = (n > BUFSIZ) ? BUFSIZ : n;
			writef(buffer, 1, blocksize);
			n -= blocksize;
		}
	}
}


/* Macros from modules/src/object/obj.h */
#define Xchar(ch)	((ch) & 0377)
#define uget2(c)	(Xchar((c)[0]) | ((unsigned) Xchar((c)[1]) << 8))
#define get4(c)		(uget2(c) | ((long) uget2((c)+2) << 16))

/* Read the ack.out file header. */

int rhead(FILE* f, struct outhead* head)
{
	char buf[SZ_HEAD], *c;
	
	if (fread(buf, sizeof(buf), 1, f) != 1)
		return 0;

	c = buf;
	head->oh_magic = uget2(c); c += 2;
	head->oh_stamp = uget2(c); c += 2;
	head->oh_flags = uget2(c); c += 2;
	head->oh_nsect = uget2(c); c += 2;
	head->oh_nrelo = uget2(c); c += 2;
	head->oh_nname = uget2(c); c += 2;
	head->oh_nemit = get4(c); c += 4;
	head->oh_nchar = get4(c);
	return 1;
}

/* Read an ack.out section header. */
 
int rsect(FILE* f, struct outsect* sect)
{
	char buf[SZ_SECT], *c;
	
	if (fread(buf, sizeof(buf), 1, f) != 1)
		return 0;

	c = buf;
	sect->os_base = get4(c); c += 4;
	sect->os_size = get4(c); c += 4;
	sect->os_foff = get4(c); c += 4;
	sect->os_flen = get4(c); c += 4;
	sect->os_lign = get4(c);
	return 1 ;
}

static void overlaps(int sect)
{
	int i;
	int base = outsect[sect].os_base;
	int end = base + outsect[sect].os_size;
	for (i = 0; i < NUM_SEGMENTS; i++) {
		if (i == sect)
			continue;
		if (outsect[i].os_base >= end)
			continue;
		if (outsect[i].os_base + outsect[i].os_size <= base)
			continue;
		/* Overlap... */
		fprintf(stderr, "%s: Section %s overlaps section %s.\n",
			program, names[i], names[sect]);
	}
}

static void range(int sect)
{
	int i;
	int base = outsect[sect].os_base;
	int end = base + outsect[sect].os_size;
	if (end > 0xFFFF)
		fprintf(stderr, "%s: Section %s exeeds space 0x%X.\n",
			program, names[sect], end);

	for (i = 0; i < NUM_SEGMENTS; i++) {
		if (i == sect)
			continue;
		if (outsect[i].os_base >= end)
			continue;
		if (outsect[i].os_base + outsect[i].os_size <= base)
			continue;
		/* Overlap... */
		fprintf(stderr, "%s: Section %s overlaps section %s.\n",
			program, names[i], names[sect]);
	}
}

static uint8_t maptable[256];

static void initmap(void)
{
	memset(maptable,'-', 256);
}

static void maps(int sect)
{
	int base = (outsect[sect].os_base + 127) >> 8;
	int end = (outsect[sect].os_base + outsect[sect].os_size + 127) >> 8;
	if (end > 0xFF)
		end = 0xFF;
	memset(maptable + base, "TRDBCX"[sect], end - base);
}

static void printmap(void)
{
	int i, r;

	for (r = 0; r < 4; r++) {
		for (i = 0; i < 256; i += 4) {
			putchar(maptable[i + r]);
			if ((i & 0x3C) == 0x3C)
				putchar(' ');
		}
		putchar('\n');
	}
}

int main(int argc, char* argv[])
{
	int i;
	/* General housecleaning and setup. */
	
	input = stdin;
	output = stdout;
	program = argv[0];
	
	/* Read in and process any flags. */
	
	while ((argc > 1) && (argv[1][0] == '-'))
	{
		switch (argv[1][1])
		{
			case 'h':
				fprintf(stderr, "%s: Syntax: aslod [-h] <inputfile> <outputfile>\n",
					program);
				exit(0);
				
			case 'v':
				verbose = true;
				break;

			default:
			syntaxerror:
				fatal("syntax error --- try -h for help");
		}
		
		argv++;
		argc--;
	}

	/* Process the rest of the arguments. */
	
	switch (argc)
	{
		case 1: /* No parameters --- read from stdin, write to stdout. */
			break;
			
		case 3: /* Both input and output files specified. */
			output = fopen(argv[2], "w");
			if (!output)
				fatal("unable to open output file.");
			outputfile = argv[2];
			/* fall through */
			
		case 2: /* Input file specified. */
			input = fopen(argv[1], "r");
			if (!input)
				fatal("unable to open input file.");
			break;
			
		default:
			goto syntaxerror;
	}

	/* Read and check the ack.out file header. */
				
	if (!rhead(input,&outhead))
		fatal("failed to read file header.");
	if (BADMAGIC(outhead))
		fatal("this isn't an ack object file.");
	if (outhead.oh_nrelo > 0)
		fprintf(stderr, "Warning: relocation information present.");
	if (!((outhead.oh_nsect == NUM_SEGMENTS) ||
	      (outhead.oh_nsect == (NUM_SEGMENTS+1))))
		fatal("the input file must have %d sections, not %ld.",
			NUM_SEGMENTS, outhead.oh_nsect);
			
	/* Read in the section headers. */
	
	for (i=0; i<outhead.oh_nsect; i++)
	{
		if (!rsect(input, &outsect[i]))
			fatal("failed to read section header %d.", i);
	}

	/* A few checks */

	if (outsect[BSS].os_flen != 0)
		fatal("the bss space contains initialized data.");

	if (!follows(&outsect[BSS], &outsect[DATA]))
		fatal("the bss segment must follow the data segment.");

	if (!follows(& outsect[ROM], &outsect[TEXT]))
		fatal("the rom segment must follow the text segment.");

	if (!follows(&outsect[DATA], &outsect[ROM]))
		fatal("the data segment must follow the rom segment.") ;

	for (i = 0; i < NUM_SEGMENTS; i++) {
		overlaps(i);
		range(i);
	}

	initmap();

	for (i = 0; i < NUM_SEGMENTS; i++)
		maps(i);

	emits(&outsect[TEXT]);
	emits(&outsect[ROM]);
	emits(&outsect[DATA]);
	emits(&outsect[COMMON]);
	emits(&outsect[DISCARD]);

	if (ferror(output))
		fatal("output write error");

	/* Summarise what we've done. */
	
	if (verbose)
	{
		uint32_t ss = 0;
		printf(" base     : %08"PRIx32"\n", outsect[TEXT].os_base) ;
		printf(" text     = %08"PRIx32"\n", outsect[TEXT].os_size);
		printf(" rom      = %08"PRIx32"\n", outsect[ROM].os_size);
		printf(" data     = %08"PRIx32"\n", outsect[DATA].os_size);
		printf(" bss      = %08"PRIx32"\n", outsect[BSS].os_size);
		printf(" common   = %08"PRIx32"\n", outsect[COMMON].os_size);
		printf(" discard  = %08"PRIx32"\n", outsect[DISCARD].os_size);
		ss += outsect[TEXT].os_size;
		ss += outsect[ROM].os_size;
		ss += outsect[DATA].os_size;
		ss += outsect[BSS].os_size;
		ss += outsect[COMMON].os_size;
		ss += outsect[DISCARD].os_size;
		printf("TOTAL  = %08"PRIx32"\n", ss);
		printf("PACKED = %08"PRIx32"\n", ss - outsect[BSS].os_size);
		printmap();
	}
	
	return 0;
}

