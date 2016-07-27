/* table.c - table-handler module for linker */

/* Copyright (C) 1994 Bruce Evans */

#include "syshead.h"
#include "const.h"
#include "align.h"
#include "obj.h"
#include "type.h"
#include "globvar.h"

#define GOLDEN		157	/* GOLDEN/HASHTABSIZE approx golden ratio */
#define HASHTABSIZE	256

static struct symstruct *hashtab[HASHTABSIZE];	/* hash table */
static char *tableptr;		/* next free spot in catchall table */
static char *tableend;		/* ptr to spot after last in table */

int maxused = 0;	/* Stats */
static int mainavail, usedtop;	/* Stats */

static struct symstruct **gethashptr(char *name);
static void check_used(void);

/* initialise symbol table */

void syminit(void)
{
    unsigned i;

    /* FIXME: use different methods for different system sizes */
#ifdef MEM_SMALL
    i = (unsigned char *)&i - (unsigned char *)sbrk(0) - 512;
    if (i >= 0xFDFF)
        outofmemory();
    i &= 0xFE00;
#else
    i = sizeof(int) <= 2 ? 0xE000 : (unsigned) 0x38000;
#endif
    for (; i != 0; i -= 512)
	if ((tableptr = malloc(i)) != NUL_PTR)
	    break;
    if (tableptr == NUL_PTR)
	outofmemory();
    tableend = tableptr + i;
    for (i = 0; i < HASHTABSIZE; i++)
	hashtab[i] = NUL_PTR;

    mainavail = tableend - tableptr;
    usedtop = 0;
}

/* add named symbol to end of table - initialise only name and next fields */
/* caller must not duplicate names of externals for findsym() to work */

struct symstruct *addsym(char *name)
{
    struct symstruct **hashptr;
    struct symstruct *oldsymptr = 0;
    struct symstruct *symptr;

    hashptr = gethashptr(name);
    symptr = *hashptr;
    while (symptr != NUL_PTR)
    {
	oldsymptr = symptr;
	symptr = symptr->next;
    }
    align(tableptr);
    symptr = (struct symstruct *) tableptr;
    if ((tableptr = symptr->name + (strlen(name) + 1)) > tableend)
	outofmemory();
    symptr->modptr = NUL_PTR;
    symptr->next = NUL_PTR;
    if (name != symptr->name)
	strcpy(symptr->name, name);	/* should't happen */
    if (*hashptr == NUL_PTR)
	*hashptr = symptr;
    else
	oldsymptr->next = symptr;
    return symptr;
}

/* lookup named symbol */

struct symstruct *findsym(char *name)
{
    struct symstruct *symptr;

    symptr = *gethashptr(name);
    while (symptr != NUL_PTR && (!(symptr->flags & (E_MASK | I_MASK)) ||
			      strcmp(symptr->name, name) != 0))
	symptr = symptr->next;
    return symptr;
}

/* convert name to a hash table ptr */

static struct symstruct **gethashptr(char *name)
{
    register unsigned hashval;

    hashval = 0;
    while (*name)
	hashval = hashval * 2 + *name++;
    return hashtab + ((hashval * GOLDEN) & (HASHTABSIZE - 1));

/*

#asm

GOLDEN	EQU	157
HASHTABSIZE	EQU	256

	CLRB		can build value here since HASHTABSIZE <= 256
	LDA	,X
	BEQ	HASHVAL.EXIT
HASHVAL.LOOP
	ADDB	,X+
	LSLB
	LDA	,X
	BNE	HASHVAL.LOOP
	RORB
	LDA	#GOLDEN
	MUL
HASHVAL.EXIT
HASHVAL.EXIT
	LDX	#_hashtab
	ABX			discard	A - same as taking mod HASHTABSIZE
	ABX
#endasm

*/

}

/* move symbol descriptor entries to top of table (no error checking) */

char *moveup(unsigned nbytes)
{
    register char *source;
    register char *target;

    usedtop   += nbytes;
    mainavail -= nbytes;

    source = tableptr;
    target = tableend;
    while (nbytes--)
	*--target = *--source;
    tableptr = source;
    return tableend = target;
}

/* our version of malloc */

char *ourmalloc(unsigned nbytes)
{
    char *allocptr;

    align(tableptr);
    allocptr = tableptr;
    if ((tableptr += nbytes) > tableend)
	outofmemory();
    return allocptr;
}

/* our version of free (release from bottom of table) */

void ourfree(char *cptr)
{
    check_used();
    tableptr = cptr;
    check_used();
}

/* read string from file into table at offset suitable for next symbol */

char *readstring(void)
{
    int c;
    char *s;
    char *start;

    align(tableptr);
    start = s = ((struct symstruct *) tableptr)->name;
    while (TRUE)
    {
        /* Stats: need a checkused against 's', maybe. */
	if (s >= tableend)
	    outofmemory();
	if ((c = readchar()) < 0)
	    prematureeof();
	if ((*s++ = c) == 0)
	    return start;
    }
    /* NOTREACHED */
}

/* release from top of table */

void release(char *cptr)
{
    check_used();
    mainavail += cptr - tableend;
    usedtop -= cptr - tableend;

    tableend = cptr;
}

static void check_used(void)
{
   int used;

   used = usedtop + mainavail - (tableend - tableptr);
   if (used > maxused) maxused = used;
}

int memory_used(void)
{
   check_used();
   return maxused;
}

/* allocate space for string */

char *stralloc(char *s)
{
    return strcpy(ourmalloc((unsigned) strlen(s) + 1), s);
}
