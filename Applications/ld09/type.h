/* type.h - types for linker */

/* Copyright (C) 1994 Bruce Evans */

typedef int bool_pt;
typedef unsigned char bool_t;

typedef unsigned short u2_t;
typedef unsigned u2_pt;
typedef unsigned long u4_t;
typedef unsigned long u4_pt;

#ifdef HOST_8BIT
typedef char fastin_t;
#else
typedef int fastin_t;
#endif
typedef int fastin_pt;

typedef unsigned flags_t;	/* unsigned makes shifts logical */

#ifdef LONG_OFFSETS
typedef unsigned long bin_off_t;
#else
typedef unsigned bin_off_t;
#endif

#ifdef OBJ_H			/* obj.h is included */

struct entrylist		/* list of entry symbols */
{
    struct entrylist *elnext;	/* next on list */
    struct symstruct *elsymptr;	/* entry on list */
};

struct modstruct		/* module table entry format */
{
    char *filename;		/* file containing this module */
    char *archentry;		/* name of component file for archives */
    char *modname;		/* name of module */
    unsigned long textoffset;	/* offset to module text in file */
    char class;			/* class of module */
    char loadflag;		/* set if module to be loaded */
    char segmaxsize[NSEG / 4];	/* |SF|SE|..|S0|, 2 bits for seg max size */
				/* 00 = 1, 01 = 2, 10 = 3, 11 = 4 */
    char segsizedesc[NSEG / 4];	/* |SF|SE|..|S0|, 2 bits for #bytes for size */
				/* 00 = 0, 01 = 1, 10 = 2, 11 = 4 */
    struct symstruct **symparray;	/* ^array of ptrs to referenced syms */
    struct modstruct *modnext;	/* next module in order of initial reading */
    char segsize[1];		/* up to 64 size bytes begin here */
};				/* careful with sizeof( struct modstruct )!! */

struct redlist			/* list of redefined (exported) symbols */
{
    struct redlist *rlnext;	/* next on list */
    struct symstruct *rlsymptr;	/* to symbol with same name, flags */
    struct modstruct *rlmodptr;	/* module for this redefinition */
    bin_off_t rlvalue;		/* value for this redefinition */
};

struct symstruct		/* symbol table entry format */
{
    struct modstruct *modptr;	/* module where symbol is defined */
    bin_off_t value;		/* value of symbol */
    flags_t flags;		/* see below (unsigned makes shifts logical) */
    struct symstruct *next;	/* next symbol with same hash value */
    char name[1];		/* name is any string beginning here */
};				/* don't use sizeof( struct symstruct )!! */

#endif				/* obj.h is included */

/* prototypes */

/* dump.c */
void dumpmods(void);
void dumpsyms(void);

/* io.c */
void ioinit(const char *progname);
void closein(void);
void closeout(void);
void errtrace(char *name, int level);
void executable(void);
void flusherr(void);
void openin(char *filename);
void openout(char *filename);
void putstr(const char *message);
void put08x(bin_off_t num);
void put08lx(bin_off_t num);
void putbstr(unsigned width, char *str);
void putbyte(int ch);
int readchar(void);
void readin(char *buf, unsigned count);
bool_pt readineofok(char *buf, unsigned count);
void seekin(unsigned long offset);
void seekout(unsigned long offset);
void seektrel(unsigned long offset);
void writechar(int c);
void writedrel(char *buf, unsigned count);
void writeout(char *buf, unsigned count);
void writetrel(char *buf, unsigned count);
void fatalerror(char *message);
void inputerror(char *message);
void input1error(char *message);
void outofmemory(void);
void prematureeof(void);
void redefined(char *name, char *message, char *archentry,
		  char *deffilename, char *defarchentry);
void interseg(char *fname, char *aname, char *name);
void reserved(char *name);
void size_error(int seg, bin_off_t count, bin_off_t size);
void undefined(char *name);
void usage(void);
void version_msg(void);
void use_error(char *message);

/* ld.c */
int main(int argc, char **argv);

/* readobj.c */
void objinit(void);
void readsyms(char *filename, bool_pt trace);
#ifdef OBJ_H
void entrysym(struct symstruct *symptr);
unsigned segsizecount(unsigned seg, struct modstruct *modptr);
#endif
bin_off_t readconvsize(unsigned countindex);
bin_off_t readsize(unsigned count);

/* table.c */
void syminit(void);
struct symstruct *addsym(char *name);
struct symstruct *findsym(char *name);
char *moveup(unsigned nbytes);
char *ourmalloc(unsigned nbytes);
void ourfree(char *cptr);
char *readstring(void);
void release(char *cptr);
int memory_used(void);
char *stralloc(char *s);

/* typeconvert.c */
u2_pt c2u2(char *buf);
u4_t c4u4(char *buf);
u2_pt cnu2(char *buf, unsigned count);
u4_t cnu4(char *buf, unsigned count);
void u2c2(char *buf, u2_pt offset);
void u4c4(char *buf, u4_t offset);
void u2cn(char *buf, u2_pt offset, unsigned count);
void u4cn(char *buf, u4_t offset, unsigned count);
bool_pt typeconv_init(bool_pt big_endian, bool_pt long_big_endian);

/* writebin.c */
void writebin(char *outfilename, bool_pt argsepid, bool_pt argbits32,
		 bool_pt argstripflag, bool_pt arguzp);

void write_dosemu(char *outfilename, bool_pt argsepid, bool_pt argbits32,
		 bool_pt argstripflag, bool_pt arguzp);

/* write_elks.c */
void write_elks(char *outfilename, bool_pt argsepid, bool_pt argbits32,
		 bool_pt argstripflag, bool_pt arguzp, bool_pt nsym);

/* write_fuzix.c */
void write_fuzix(char *outfilename, bool_pt argsepid, bool_pt argbits32,
		 bool_pt argstripflag, bool_pt arguzp);

/* linksym.c */
void linksyms(bool_pt argreloc_output);

/* mkar.c */
void ld86r(int argc, char ** argv);
